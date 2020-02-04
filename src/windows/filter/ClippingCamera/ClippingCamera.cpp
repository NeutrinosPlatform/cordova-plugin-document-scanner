// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.

#include "ClippingCamera.h"
#include "bufferlock.h"

#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\features2d\features2d.hpp>
#include <opencv2\imgcodecs.hpp>



using namespace Microsoft::WRL;

/*

This sample implements a video effect as a Media Foundation transform (MFT).

NOTES ON THE MFT IMPLEMENTATION

1. The MFT has fixed streams: One input stream and one output stream.

2. The MFT supports NV12 format only.

3. If the MFT is holding an input sample, SetInputType and SetOutputType both fail.

4. The input and output types must be identical.

5. If both types are set, no type can be set until the current type is cleared.

6. Preferred input types:

     (a) If the output type is set, that's the preferred type.
     (b) Otherwise, the preferred types are partial types, constructed from the
         list of supported subtypes.

7. Preferred output types: As above.

8. Streaming:

    The private BeingStreaming() method is called in response to the
    MFT_MESSAGE_NOTIFY_BEGIN_STREAMING message.

    If the client does not send MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, the MFT calls
    BeginStreaming inside the first call to ProcessInput or ProcessOutput.

    This is a good approach for allocating resources that your MFT requires for
    streaming.

9. The configuration attributes are applied in the BeginStreaming method. If the
   client changes the attributes during streaming, the change is ignored until
   streaming is stopped (either by changing the media types or by sending the
   MFT_MESSAGE_NOTIFY_END_STREAMING message) and then restarted.

*/


// Static array of media types (preferred and accepted).
const GUID g_MediaSubtypes[] =
{
    MFVideoFormat_NV12
};

HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride);

template <typename T>
inline T clamp(const T& val, const T& minVal, const T& maxVal)
{
    return (val < minVal ? minVal : (val > maxVal ? maxVal : val));
}

ImageClipping::ImageClipping() :
	m_pSample(NULL), m_pInputType(NULL), m_pOutputType(NULL),
	m_imageWidthInPixels(0), m_imageHeightInPixels(0), m_cbImageSize(0),
	m_bStreamingInitialized(false),
	m_pAttributes(NULL), m_pSetting(NULL),
	m_bDoClip(false), m_nQuality(100), m_bConvertToGrayscale(false), m_nRotation(0)
{
	m_prevPoints.clear();
	InitializeCriticalSectionEx(&m_critSec, 3000, 0);
}

ImageClipping::~ImageClipping()
{
    SafeRelease(&m_pInputType);
    SafeRelease(&m_pOutputType);
    SafeRelease(&m_pSample);
    SafeRelease(&m_pAttributes);
    DeleteCriticalSection(&m_critSec);
}

// Initialize the instance.
STDMETHODIMP ImageClipping::RuntimeClassInitialize()
{
    // Create the attribute store.
    return MFCreateAttributes(&m_pAttributes, 3);
}

// IMediaExtension methods

//-------------------------------------------------------------------
// SetProperties
// Sets the configuration of the effect
//-------------------------------------------------------------------
HRESULT ImageClipping::SetProperties(ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration)
{
    HRESULT hr = S_OK;

    if (!pConfiguration)
        return hr;

    //ComPtr<ABI::Windows::Foundation::Collections::IMap<HSTRING, IInspectable *>> spSetting;
    pConfiguration->QueryInterface(IID_PPV_ARGS(&m_pSetting));


	HSTRING key;
	boolean found;
	ComPtr<ABI::Windows::Foundation::IPropertyValue> spPropVal;
	ComPtr<IInspectable> spInsp;

	// INT32 quality
	WindowsCreateString(L"{698649BE-8EAE-4551-A4CB-2FA79A4E1E70}", 38, &key);
	m_pSetting->HasKey(key, &found);
	if (found) {
		m_pSetting->Lookup(key, spInsp.ReleaseAndGetAddressOf());
		hr = spInsp.As(&spPropVal);
		if (hr != S_OK) {
			return hr;
		}
		hr = spPropVal->GetUInt32(&m_nQuality);
		if (hr != S_OK) {
			return hr;
		}
	}

	// bool bConvertToGrayscale
	WindowsCreateString(L"{698649BE-8EAE-4551-A4CB-2FA79A4E1E71}", 38, &key);
	m_pSetting->HasKey(key, &found);
	if (found) {
		m_pSetting->Lookup(key, spInsp.ReleaseAndGetAddressOf());
		hr = spInsp.As(&spPropVal);
		if (hr != S_OK) {
			return hr;
		}
		hr = spPropVal->GetBoolean(&m_bConvertToGrayscale);
		if (hr != S_OK) {
			return hr;
		}
	}

	// INT32 rotation
	WindowsCreateString(L"{698649BE-8EAE-4551-A4CB-2FA79A4E1E72}", 38, &key);
	m_pSetting->HasKey(key, &found);
	if (found) {
		m_pSetting->Lookup(key, spInsp.ReleaseAndGetAddressOf());
		hr = spInsp.As(&spPropVal);
		if (hr != S_OK) {
			return hr;
		}
		hr = spPropVal->GetUInt32(&m_nRotation);
		if (hr != S_OK) {
			return hr;
		}
	}

	// bool bDoCapture
	WindowsCreateString(L"{698649BE-8EAE-4551-A4CB-2FA79A4E1E79}", 38, &key);
	m_pSetting->HasKey(key, &found);
    if (found) {
		m_pSetting->Lookup(key, spInsp.ReleaseAndGetAddressOf());
        hr = spInsp.As(&spPropVal);
        if (hr != S_OK)  {
            return hr;
        }
        hr = spPropVal->GetBoolean(&m_bDoClip);
        if (hr != S_OK) {
            return hr;
        }
    }

	return hr;
}

// IMFTransform methods. Refer to the Media Foundation SDK documentation for details.

//-------------------------------------------------------------------
// GetStreamLimits
// Returns the minimum and maximum number of streams.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetStreamLimits(
    DWORD   *pdwInputMinimum,
    DWORD   *pdwInputMaximum,
    DWORD   *pdwOutputMinimum,
    DWORD   *pdwOutputMaximum
)
{
    if ((pdwInputMinimum == NULL) ||
        (pdwInputMaximum == NULL) ||
        (pdwOutputMinimum == NULL) ||
        (pdwOutputMaximum == NULL))
    {
        return E_POINTER;
    }

    // This MFT has a fixed number of streams.
    *pdwInputMinimum = 1;
    *pdwInputMaximum = 1;
    *pdwOutputMinimum = 1;
    *pdwOutputMaximum = 1;
    return S_OK;
}


//-------------------------------------------------------------------
// GetStreamCount
// Returns the actual number of streams.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetStreamCount(
    DWORD   *pcInputStreams,
    DWORD   *pcOutputStreams
)
{
    if ((pcInputStreams == NULL) || (pcOutputStreams == NULL))

    {
        return E_POINTER;
    }

    // This MFT has a fixed number of streams.
    *pcInputStreams = 1;
    *pcOutputStreams = 1;
    return S_OK;
}



//-------------------------------------------------------------------
// GetStreamIDs
// Returns stream IDs for the input and output streams.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetStreamIDs(
    DWORD   dwInputIDArraySize,
    DWORD   *pdwInputIDs,
    DWORD   dwOutputIDArraySize,
    DWORD   *pdwOutputIDs
)
{
    // It is not required to implement this method if the MFT has a fixed number of
    // streams AND the stream IDs are numbered sequentially from zero (that is, the
    // stream IDs match the stream indexes).

    // In that case, it is OK to return E_NOTIMPL.
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// GetInputStreamInfo
// Returns information about an input stream.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetInputStreamInfo(
    DWORD                     dwInputStreamID,
    MFT_INPUT_STREAM_INFO *   pStreamInfo
)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_critSec);

    if (!IsValidInputStream(dwInputStreamID))
    {
        LeaveCriticalSection(&m_critSec);
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // NOTE: This method should succeed even when there is no media type on the
    //       stream. If there is no media type, we only need to fill in the dwFlags
    //       member of MFT_INPUT_STREAM_INFO. The other members depend on having a
    //       a valid media type.

    pStreamInfo->hnsMaxLatency = 0;
    pStreamInfo->dwFlags = MFT_INPUT_STREAM_WHOLE_SAMPLES | MFT_INPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER;

    if (m_pInputType == NULL)
    {
        pStreamInfo->cbSize = 0;
    }
    else
    {
        pStreamInfo->cbSize = m_cbImageSize;
    }

    pStreamInfo->cbMaxLookahead = 0;
    pStreamInfo->cbAlignment = 0;

    LeaveCriticalSection(&m_critSec);
    return S_OK;
}

//-------------------------------------------------------------------
// GetOutputStreamInfo
// Returns information about an output stream.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetOutputStreamInfo(
    DWORD                     dwOutputStreamID,
    MFT_OUTPUT_STREAM_INFO *  pStreamInfo
)
{
    if (pStreamInfo == NULL)
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_critSec);

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        LeaveCriticalSection(&m_critSec);
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // NOTE: This method should succeed even when there is no media type on the
    //       stream. If there is no media type, we only need to fill in the dwFlags
    //       member of MFT_OUTPUT_STREAM_INFO. The other members depend on having a
    //       a valid media type.

    pStreamInfo->dwFlags =
        MFT_OUTPUT_STREAM_WHOLE_SAMPLES |
        MFT_OUTPUT_STREAM_SINGLE_SAMPLE_PER_BUFFER |
        MFT_OUTPUT_STREAM_FIXED_SAMPLE_SIZE ;

    if (m_pOutputType == NULL)
    {
        pStreamInfo->cbSize = 0;
    }
    else
    {
        pStreamInfo->cbSize = m_cbImageSize;
    }

    pStreamInfo->cbAlignment = 0;

    LeaveCriticalSection(&m_critSec);
    return S_OK;
}


//-------------------------------------------------------------------
// GetAttributes
// Returns the attributes for the MFT.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetAttributes(IMFAttributes** ppAttributes)
{
    if (ppAttributes == NULL)
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_critSec);

    *ppAttributes = m_pAttributes;
    (*ppAttributes)->AddRef();

    LeaveCriticalSection(&m_critSec);
    return S_OK;
}


//-------------------------------------------------------------------
// GetInputStreamAttributes
// Returns stream-level attributes for an input stream.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetInputStreamAttributes(
    DWORD           dwInputStreamID,
    IMFAttributes   **ppAttributes
)
{
    // This MFT does not support any stream-level attributes, so the method is not implemented.
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// GetOutputStreamAttributes
// Returns stream-level attributes for an output stream.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetOutputStreamAttributes(
    DWORD           dwOutputStreamID,
    IMFAttributes   **ppAttributes
)
{
    // This MFT does not support any stream-level attributes, so the method is not implemented.
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// DeleteInputStream
//-------------------------------------------------------------------

HRESULT ImageClipping::DeleteInputStream(DWORD dwStreamID)
{
    // This MFT has a fixed number of input streams, so the method is not supported.
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// AddInputStreams
//-------------------------------------------------------------------

HRESULT ImageClipping::AddInputStreams(
    DWORD   cStreams,
    DWORD   *adwStreamIDs
)
{
    // This MFT has a fixed number of output streams, so the method is not supported.
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// GetInputAvailableType
// Returns a preferred input type.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetInputAvailableType(
    DWORD           dwInputStreamID,
    DWORD           dwTypeIndex, // 0-based
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&m_critSec);

    if (!IsValidInputStream(dwInputStreamID))
    {
        LeaveCriticalSection(&m_critSec);
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr = S_OK;

    // If the output type is set, return that type as our preferred input type.
    if (m_pOutputType == NULL)
    {
        // The output type is not set. Create a partial media type.
        hr = OnGetPartialType(dwTypeIndex, ppType);
    }
    else if (dwTypeIndex > 0)
    {
        hr = MF_E_NO_MORE_TYPES;
    }
    else
    {
        *ppType = m_pOutputType;
        (*ppType)->AddRef();
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}



//-------------------------------------------------------------------
// GetOutputAvailableType
// Returns a preferred output type.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetOutputAvailableType(
    DWORD           dwOutputStreamID,
    DWORD           dwTypeIndex, // 0-based
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&m_critSec);

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        LeaveCriticalSection(&m_critSec);
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr = S_OK;

    if (m_pInputType == NULL)
    {
        // The input type is not set. Create a partial media type.
        hr = OnGetPartialType(dwTypeIndex, ppType);
    }
    else if (dwTypeIndex > 0)
    {
        hr = MF_E_NO_MORE_TYPES;
    }
    else
    {
        *ppType = m_pInputType;
        (*ppType)->AddRef();
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}


//-------------------------------------------------------------------
// SetInputType
//-------------------------------------------------------------------

HRESULT ImageClipping::SetInputType(
    DWORD           dwInputStreamID,
    IMFMediaType    *pType, // Can be NULL to clear the input type.
    DWORD           dwFlags
)
{
    // Validate flags.
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&m_critSec);

    if (!IsValidInputStream(dwInputStreamID))
    {
        LeaveCriticalSection(&m_critSec);
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr = S_OK;

    // Does the caller want us to set the type, or just test it?
    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

    // If we have an input sample, the client cannot change the type now.
    if (HasPendingOutput())
    {
        hr = MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
        goto done;
    }

    // Validate the type, if non-NULL.
    if (pType)
    {
        hr = OnCheckInputType(pType);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    // The type is OK. Set the type, unless the caller was just testing.
    if (bReallySet)
    {
        OnSetInputType(pType);

        // When the type changes, end streaming.
        hr = EndStreaming();
    }

done:
    LeaveCriticalSection(&m_critSec);
    return hr;
}



//-------------------------------------------------------------------
// SetOutputType
//-------------------------------------------------------------------

HRESULT ImageClipping::SetOutputType(
    DWORD           dwOutputStreamID,
    IMFMediaType    *pType, // Can be NULL to clear the output type.
    DWORD           dwFlags
)
{
    // Validate flags.
    if (dwFlags & ~MFT_SET_TYPE_TEST_ONLY)
    {
        return E_INVALIDARG;
    }

    EnterCriticalSection(&m_critSec);

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        LeaveCriticalSection(&m_critSec);
        return MF_E_INVALIDSTREAMNUMBER;
    }

    HRESULT hr = S_OK;

    // Does the caller want us to set the type, or just test it?
    BOOL bReallySet = ((dwFlags & MFT_SET_TYPE_TEST_ONLY) == 0);

    // If we have an input sample, the client cannot change the type now.
    if (HasPendingOutput())
    {
        hr = MF_E_TRANSFORM_CANNOT_CHANGE_MEDIATYPE_WHILE_PROCESSING;
        goto done;
    }

    // Validate the type, if non-NULL.
    if (pType)
    {
        hr = OnCheckOutputType(pType);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    // The type is OK. Set the type, unless the caller was just testing.
    if (bReallySet)
    {
        OnSetOutputType(pType);

        // When the type changes, end streaming.
        hr = EndStreaming();
    }

done:
    LeaveCriticalSection(&m_critSec);
    return hr;
}


//-------------------------------------------------------------------
// GetInputCurrentType
// Returns the current input type.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetInputCurrentType(
    DWORD           dwInputStreamID,
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    if (!IsValidInputStream(dwInputStreamID))
    {
        hr = MF_E_INVALIDSTREAMNUMBER;
    }
    else if (!m_pInputType)
    {
        hr = MF_E_TRANSFORM_TYPE_NOT_SET;
    }
    else
    {
        *ppType = m_pInputType;
        (*ppType)->AddRef();
    }
    LeaveCriticalSection(&m_critSec);
    return hr;
}


//-------------------------------------------------------------------
// GetOutputCurrentType
// Returns the current output type.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetOutputCurrentType(
    DWORD           dwOutputStreamID,
    IMFMediaType    **ppType
)
{
    if (ppType == NULL)
    {
        return E_POINTER;
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    if (!IsValidOutputStream(dwOutputStreamID))
    {
        hr = MF_E_INVALIDSTREAMNUMBER;
    }
    else if (!m_pOutputType)
    {
        hr = MF_E_TRANSFORM_TYPE_NOT_SET;
    }
    else
    {
        *ppType = m_pOutputType;
        (*ppType)->AddRef();
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}


//-------------------------------------------------------------------
// GetInputStatus
// Query if the MFT is accepting more input.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetInputStatus(
    DWORD           dwInputStreamID,
    DWORD           *pdwFlags
)
{
    if (pdwFlags == NULL)
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_critSec);

    if (!IsValidInputStream(dwInputStreamID))
    {
        LeaveCriticalSection(&m_critSec);
        return MF_E_INVALIDSTREAMNUMBER;
    }

    // If an input sample is already queued, do not accept another sample until the
    // client calls ProcessOutput or Flush.

    // NOTE: It is possible for an MFT to accept more than one input sample. For
    // example, this might be required in a video decoder if the frames do not
    // arrive in temporal order. In the case, the decoder must hold a queue of
    // samples. For the video effect, each sample is transformed independently, so
    // there is no reason to queue multiple input samples.

    if (m_pSample == NULL)
    {
        *pdwFlags = MFT_INPUT_STATUS_ACCEPT_DATA;
    }
    else
    {
        *pdwFlags = 0;
    }

    LeaveCriticalSection(&m_critSec);
    return S_OK;
}



//-------------------------------------------------------------------
// GetOutputStatus
// Query if the MFT can produce output.
//-------------------------------------------------------------------

HRESULT ImageClipping::GetOutputStatus(DWORD *pdwFlags)
{
    if (pdwFlags == NULL)
    {
        return E_POINTER;
    }

    EnterCriticalSection(&m_critSec);

    // The MFT can produce an output sample if (and only if) there an input sample.
    if (m_pSample != NULL)
    {
        *pdwFlags = MFT_OUTPUT_STATUS_SAMPLE_READY;
    }
    else
    {
        *pdwFlags = 0;
    }

    LeaveCriticalSection(&m_critSec);
    return S_OK;
}


//-------------------------------------------------------------------
// SetOutputBounds
// Sets the range of time stamps that the MFT will output.
//-------------------------------------------------------------------

HRESULT ImageClipping::SetOutputBounds(
    LONGLONG        hnsLowerBound,
    LONGLONG        hnsUpperBound
)
{
    // Implementation of this method is optional.
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// ProcessEvent
// Sends an event to an input stream.
//-------------------------------------------------------------------

HRESULT ImageClipping::ProcessEvent(
    DWORD              dwInputStreamID,
    IMFMediaEvent      *pEvent
)
{
    // This MFT does not handle any stream events, so the method can
    // return E_NOTIMPL. This tells the pipeline that it can stop
    // sending any more events to this MFT.
    return E_NOTIMPL;
}


//-------------------------------------------------------------------
// ProcessMessage
//-------------------------------------------------------------------

HRESULT ImageClipping::ProcessMessage(
    MFT_MESSAGE_TYPE    eMessage,
    ULONG_PTR           ulParam
)
{
    EnterCriticalSection(&m_critSec);

    HRESULT hr = S_OK;

    switch (eMessage)
    {
    case MFT_MESSAGE_COMMAND_FLUSH:
        // Flush the MFT.
        hr = OnFlush();
        break;

    case MFT_MESSAGE_COMMAND_DRAIN:
        // Drain: Tells the MFT to reject further input until all pending samples are
        // processed. That is our default behavior already, so there is nothing to do.
        //
        // For a decoder that accepts a queue of samples, the MFT might need to drain
        // the queue in response to this command.
    break;

    case MFT_MESSAGE_SET_D3D_MANAGER:
        // Sets a pointer to the IDirect3DDeviceManager9 interface.

        // The pipeline should never send this message unless the MFT sets the MF_SA_D3D_AWARE
        // attribute set to TRUE. Because this MFT does not set MF_SA_D3D_AWARE, it is an error
        // to send the MFT_MESSAGE_SET_D3D_MANAGER message to the MFT. Return an error code in
        // this case.

        // NOTE: If this MFT were D3D-enabled, it would cache the IDirect3DDeviceManager9
        // pointer for use during streaming.

        hr = E_NOTIMPL;
        break;

    case MFT_MESSAGE_NOTIFY_BEGIN_STREAMING:
        hr = BeginStreaming();
        break;

    case MFT_MESSAGE_NOTIFY_END_STREAMING:
        hr = EndStreaming();
        break;

    // The next two messages do not require any action from this MFT.

    case MFT_MESSAGE_NOTIFY_END_OF_STREAM:
        break;

    case MFT_MESSAGE_NOTIFY_START_OF_STREAM:
        break;
    }

    LeaveCriticalSection(&m_critSec);
    return hr;
}


//-------------------------------------------------------------------
// ProcessInput
// Process an input sample.
//-------------------------------------------------------------------

HRESULT ImageClipping::ProcessInput(
    DWORD               dwInputStreamID,
    IMFSample           *pSample,
    DWORD               dwFlags
)
{
    // Check input parameters.
    if (pSample == NULL)
    {
        return E_POINTER;
    }

    if (dwFlags != 0)
    {
        return E_INVALIDARG; // dwFlags is reserved and must be zero.
    }

    HRESULT hr = S_OK;

    EnterCriticalSection(&m_critSec);

    // Validate the input stream number.
    if (!IsValidInputStream(dwInputStreamID))
    {
        hr = MF_E_INVALIDSTREAMNUMBER;
        goto done;
    }

    // Check for valid media types.
    // The client must set input and output types before calling ProcessInput.
    if (!m_pInputType || !m_pOutputType)
    {
        hr = MF_E_NOTACCEPTING;
        goto done;
    }

    // Check if an input sample is already queued.
    if (m_pSample != NULL)
    {
        hr = MF_E_NOTACCEPTING;   // We already have an input sample.
        goto done;
    }

    // Initialize streaming.
    hr = BeginStreaming();
    if (FAILED(hr))
    {
        goto done;
    }

    // Cache the sample. We do the actual work in ProcessOutput.
    m_pSample = pSample;
    pSample->AddRef();  // Hold a reference count on the sample.

done:
    LeaveCriticalSection(&m_critSec);
    return hr;
}


//-------------------------------------------------------------------
// ProcessOutput
// Process an output sample.
//-------------------------------------------------------------------

HRESULT ImageClipping::ProcessOutput(
    DWORD                   dwFlags,
    DWORD                   cOutputBufferCount,
    MFT_OUTPUT_DATA_BUFFER  *pOutputSamples, // one per stream
    DWORD                   *pdwStatus
)
{
    // Check input parameters...

    // This MFT does not accept any flags for the dwFlags parameter.

    // The only defined flag is MFT_PROCESS_OUTPUT_DISCARD_WHEN_NO_BUFFER. This flag
    // applies only when the MFT marks an output stream as lazy or optional. But this
    // MFT has no lazy or optional streams, so the flag is not valid.

    if (dwFlags != 0)
    {
        return E_INVALIDARG;
    }

    if (pOutputSamples == NULL || pdwStatus == NULL)
    {
        return E_POINTER;
    }

    // There must be exactly one output buffer.
    if (cOutputBufferCount != 1)
    {
        return E_INVALIDARG;
    }

    // It must contain a sample.
    if (pOutputSamples[0].pSample == NULL)
    {
        return E_INVALIDARG;
    }

    HRESULT hr = S_OK;

    IMFMediaBuffer *pInput = NULL;
    IMFMediaBuffer *pOutput = NULL;

    EnterCriticalSection(&m_critSec);

    // There must be an input sample available for processing.
    if (m_pSample == NULL)
    {
        hr = MF_E_TRANSFORM_NEED_MORE_INPUT;
        goto done;
    }

    // Initialize streaming.

    hr = BeginStreaming();
    if (FAILED(hr))
    {
        goto done;
    }

    // Get the input buffer.
    hr = m_pSample->ConvertToContiguousBuffer(&pInput);
    if (FAILED(hr))
    {
        goto done;
    }

    // Get the output buffer.
    hr = pOutputSamples[0].pSample->ConvertToContiguousBuffer(&pOutput);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = OnProcessOutput(pInput, pOutput);
    if (FAILED(hr))
    {
        goto done;
    }

    // Set status flags.
    pOutputSamples[0].dwStatus = 0;
    *pdwStatus = 0;


    // Copy the duration and time stamp from the input sample, if present.

    LONGLONG hnsDuration = 0;
    LONGLONG hnsTime = 0;

    if (SUCCEEDED(m_pSample->GetSampleDuration(&hnsDuration)))
    {
        hr = pOutputSamples[0].pSample->SetSampleDuration(hnsDuration);
        if (FAILED(hr))
        {
            goto done;
        }
    }

    if (SUCCEEDED(m_pSample->GetSampleTime(&hnsTime)))
    {
        hr = pOutputSamples[0].pSample->SetSampleTime(hnsTime);
    }

done:
    SafeRelease(&m_pSample);   // Release our input sample.
    SafeRelease(&pInput);
    SafeRelease(&pOutput);
    LeaveCriticalSection(&m_critSec);
    return hr;
}

// PRIVATE METHODS

// All methods that follow are private to this MFT and are not part of the IMFTransform interface.

// Create a partial media type from our list.
//
// dwTypeIndex: Index into the list of peferred media types.
// ppmt:        Receives a pointer to the media type.

HRESULT ImageClipping::OnGetPartialType(DWORD dwTypeIndex, IMFMediaType **ppmt)
{
    if (dwTypeIndex >= ARRAYSIZE(g_MediaSubtypes))
    {
        return MF_E_NO_MORE_TYPES;
    }

    IMFMediaType *pmt = NULL;

    HRESULT hr = MFCreateMediaType(&pmt);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pmt->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr))
    {
        goto done;
    }

    hr = pmt->SetGUID(MF_MT_SUBTYPE, g_MediaSubtypes[dwTypeIndex]);
    if (FAILED(hr))
    {
        goto done;
    }

    *ppmt = pmt;
    (*ppmt)->AddRef();

done:
    SafeRelease(&pmt);
    return hr;
}


// Validate an input media type.

HRESULT ImageClipping::OnCheckInputType(IMFMediaType *pmt)
{
    assert(pmt != NULL);

    HRESULT hr = S_OK;

    // If the output type is set, see if they match.
    if (m_pOutputType != NULL)
    {
        DWORD flags = 0;
        hr = pmt->IsEqual(m_pOutputType, &flags);

        // IsEqual can return S_FALSE. Treat this as failure.
        if (hr != S_OK)
        {
            hr = MF_E_INVALIDMEDIATYPE;
        }
    }
    else
    {
        // Output type is not set. Just check this type.
        hr = OnCheckMediaType(pmt);
    }
    return hr;
}


// Validate an output media type.

HRESULT ImageClipping::OnCheckOutputType(IMFMediaType *pmt)
{
    assert(pmt != NULL);

    HRESULT hr = S_OK;

    // If the input type is set, see if they match.
    if (m_pInputType != NULL)
    {
        DWORD flags = 0;
        hr = pmt->IsEqual(m_pInputType, &flags);

        // IsEqual can return S_FALSE. Treat this as failure.
        if (hr != S_OK)
        {
            hr = MF_E_INVALIDMEDIATYPE;
        }

    }
    else
    {
        // Input type is not set. Just check this type.
        hr = OnCheckMediaType(pmt);
    }
    return hr;
}


// Validate a media type (input or output)

HRESULT ImageClipping::OnCheckMediaType(IMFMediaType *pmt)
{
    BOOL bFoundMatchingSubtype = FALSE;

    // Major type must be video.
    GUID major_type;
    HRESULT hr = pmt->GetGUID(MF_MT_MAJOR_TYPE, &major_type);
    if (FAILED(hr))
    {
        goto done;
    }

    if (major_type != MFMediaType_Video)
    {
        hr = MF_E_INVALIDMEDIATYPE;
        goto done;
    }

    // Subtype must be one of the subtypes in our global list.

    // Get the subtype GUID.
    GUID subtype;
    hr = pmt->GetGUID(MF_MT_SUBTYPE, &subtype);
    if (FAILED(hr))
    {
        goto done;
    }

    // Look for the subtype in our list of accepted types.
    for (DWORD i = 0; i < ARRAYSIZE(g_MediaSubtypes); i++)
    {
        if (subtype == g_MediaSubtypes[i])
        {
            bFoundMatchingSubtype = TRUE;
            break;
        }
    }

    if (!bFoundMatchingSubtype)
    {
        hr = MF_E_INVALIDMEDIATYPE; // The MFT does not support this subtype.
        goto done;
    }

    // Reject single-field media types.
    UINT32 interlace = MFGetAttributeUINT32(pmt, MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive);
    if (interlace == MFVideoInterlace_FieldSingleUpper  || interlace == MFVideoInterlace_FieldSingleLower)
    {
        hr = MF_E_INVALIDMEDIATYPE;
    }

done:
    return hr;
}


// Set or clear the input media type.
//
// Prerequisite: The input type was already validated.

void ImageClipping::OnSetInputType(IMFMediaType *pmt)
{
    // if pmt is NULL, clear the type.
    // if pmt is non-NULL, set the type.

    SafeRelease(&m_pInputType);
    m_pInputType = pmt;
    if (m_pInputType)
    {
        m_pInputType->AddRef();
    }

    // Update the format information.
    UpdateFormatInfo();
}


// Set or clears the output media type.
//
// Prerequisite: The output type was already validated.

void ImageClipping::OnSetOutputType(IMFMediaType *pmt)
{
    // If pmt is NULL, clear the type. Otherwise, set the type.

    SafeRelease(&m_pOutputType);
    m_pOutputType = pmt;
    if (m_pOutputType)
    {
        m_pOutputType->AddRef();
    }
}


// Initialize streaming parameters.
//
// This method is called if the client sends the MFT_MESSAGE_NOTIFY_BEGIN_STREAMING
// message, or when the client processes a sample, whichever happens first.

HRESULT ImageClipping::BeginStreaming()
{
    HRESULT hr = S_OK;

    if (!m_bStreamingInitialized)
    {
        m_bStreamingInitialized = true;
        hr = S_OK;
    }

    return hr;
}


// End streaming.

// This method is called if the client sends an MFT_MESSAGE_NOTIFY_END_STREAMING
// message, or when the media type changes. In general, it should be called whenever
// the streaming parameters need to be reset.

HRESULT ImageClipping::EndStreaming()
{
    m_bStreamingInitialized = false;
    return S_OK;
}


bool comp_horz(cv::Point2f a, cv::Point2f b)
{
	return a.x < b.x;
}
bool comp_vert(cv::Point2f a, cv::Point2f b)
{
	return a.y < b.y;
}

// Generate output data.

HRESULT ImageClipping::OnProcessOutput(IMFMediaBuffer *pIn, IMFMediaBuffer *pOut)
{
    BYTE *pDest = NULL;         // Destination buffer.
    LONG lDestStride = 0;       // Destination stride.

    BYTE *pSrc = NULL;          // Source buffer.
    LONG lSrcStride = 0;        // Source stride.

    // Helper objects to lock the buffers.
    VideoBufferLock inputLock(pIn);
    VideoBufferLock outputLock(pOut);

    // Stride if the buffer does not support IMF2DBuffer
    LONG lDefaultStride = 0;

	UINT32 width = m_imageWidthInPixels;
	UINT32 height = m_imageHeightInPixels;

    HRESULT hr = GetDefaultStride(m_pInputType, &lDefaultStride);
    if (FAILED(hr)) {
        return hr;
    }

    // Lock the input buffer.
    hr = inputLock.LockBuffer(lDefaultStride, height, &pSrc, &lSrcStride);
    if (FAILED(hr)) {
        return hr;
    }

    // Lock the output buffer.
    hr = outputLock.LockBuffer(lDefaultStride, height, &pDest, &lDestStride);
    if (FAILED(hr)) {
        return hr;
    }

    cv::Mat InputFrame(height + height/2, width, CV_8UC1, pSrc, lSrcStride);
    cv::Mat OutputFrame(height + height/2, width, CV_8UC1, pDest, lDestStride);

	InputFrame.copyTo(OutputFrame);

	cv::Mat image;
	if (m_bDoClip) {
		if (m_bConvertToGrayscale) {
			cv::cvtColor(InputFrame, image, cv::COLOR_YUV420sp2GRAY);
		} else {
			cv::cvtColor(InputFrame, image, cv::COLOR_YUV420sp2RGB);
		}
	}

	cv::Mat WorkFrame;
	cv::cvtColor(InputFrame, WorkFrame, cv::COLOR_YUV420sp2BGRA);

	double scale = 1.0;
	int max_len = 640;//1280;
	if (width > height && width > max_len) {
        scale = (double)max_len / (double)width;
	} else if (height > max_len) {
        scale = (double)max_len / (double)height;
	}
	if (scale < 1.0) {
		cv::Mat scaled;
		cv::resize(WorkFrame,scaled,cv::Size(),scale,scale,cv::INTER_NEAREST);
	    width = scaled.cols;
	    height = scaled.rows;
	    cv::blur(scaled, scaled, cv::Size(3, 3));
		WorkFrame = scaled;
	} else {
	    cv::blur(WorkFrame, WorkFrame, cv::Size(3, 3));
	}
	int w8 = width / 8;
	int h8= height / 8;
	int w16 = w8 / 2;
	int h16= h8 / 2;
	int wdth = 0, hght = 0;

	//cv::GaussianBlur(WorkFrame, WorkFrame, cv::Size(5, 5), 0);
	//cv::Canny(WorkFrame, WorkFrame, 60, 200);

	cv::Mat gray0(WorkFrame.size(), CV_8U), gray;

	std::vector<std::vector<cv::Point> > contours;
    //std::vector<std::vector<cv::Point> > squares;

	double smallest_area = (width * height) / 64.0;
	double max_area = 0;

	std::vector<cv::Point> points;
	// find squares in every color plane of the image
    for (int c = 0; c < 3; c++) {
        int ch[] = {c, 0};
        cv::mixChannels(&WorkFrame, 1, &gray0, 1, ch, 1);

		// try several threshold levels
        //const int threshold_level = 2;
        //for (int l = 0; l < threshold_level; l++) {
            // Use Canny instead of zero threshold level!
            // Canny helps to catch squares with gradient shading
            //if (l == 0) {
                cv::Canny(gray0, gray, 10, 20, 3); //
                
                // Dilate helps to remove potential holes between edge segments
                cv::dilate(gray, gray, cv::Mat(), cv::Point(-1,-1));
            //} else {
            //    gray = gray0 >= (l+1) * 255 / threshold_level;
            //}
            
            // Find contours and store them in a list
            cv::findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
            
            // Test contours
            std::vector<cv::Point> approx;
            for (size_t i = 0; i < contours.size(); i++) {
                // approximate contour with accuracy proportional
                // to the contour perimeter
				std::vector<cv::Point> hull;
				cv::convexHull(contours[i], hull, true);
				double area = fabs(cv::contourArea(hull));
				if (area > max_area && area > smallest_area) {
					double peri = cv::arcLength(hull, true);
					cv::approxPolyDP(hull, approx, 0.02 * peri, true);
					if (approx.size() == 4) {
						// sort the polygon's corners
						cv::Point swap(0, 0);
						sort(approx.begin(), approx.end(), comp_vert);
						if (approx[0].x > approx[1].x) {
							swap = approx[0];
							approx[0] = approx[1];
							approx[1] = swap;
						}
						if (approx[2].x < approx[3].x) {
							swap = approx[2];
							approx[2] = approx[3];
							approx[3] = swap;
						}
						// use only appropriate polygons
						double len1 = norm(approx[1] - approx[0]), 
							len2 = norm(approx[2] - approx[1]), 
							len3 = norm(approx[3] - approx[2]), 
							len4 = norm(approx[0] - approx[3]);
						if (len1 > w8 &&
							len2 > h8 &&
							len3 > w8 &&
							len4 > h8 &&
							abs(len1 - len3) < ((len1 + len3) / 4) &&
							abs(len2 - len4) < ((len2 + len4) / 4) &&
							(abs(width  - len1) > w16 &&
							 abs(width  - len3) > w16 ||
							 abs(height - len2) > h16 &&
							 abs(height - len4) > h16)) {

							double maxCosine = 0;
                    
							for (int j = 2; j < 5; j++) {
								double cosine = fabs(angle(approx[j%4], approx[j-2], approx[j-1]));
								maxCosine = MAX(maxCosine, cosine);
							}
                    
							if (maxCosine < 0.3) {
								points = approx;
								max_area = area;
								wdth = round((len1 + len3) / 2);
								hght = round((len2 + len4) / 2);
							}

						}
					}
				}
            }
        //}

    }
	if (points.size() == 4) {
		m_prevPoints = points;
		m_prevWdth = wdth;
		m_prevHght = hght;
	} else if (m_prevPoints.size() == 4) {
		points = m_prevPoints;
		wdth = m_prevWdth;
		hght = m_prevHght;
		m_prevPoints.clear();
		m_prevWdth = 0;
		m_prevHght = 0;
	} else {
		points.clear();
	}
	if (points.size() == 4 && wdth > 0 && hght > 0) {
		// draw approx only if it has 4 corners
		std::vector<cv::Point> approx2;
		std::vector<cv::Point2f> pointsf;

		for (size_t k = 0; k < points.size(); k++) {
			cv::Point pt2(round((double)points[k].x / scale / 2.0), round((double)points[k].y / scale / 2.0));
			approx2.insert(approx2.begin(), pt2);
			cv::Point2f pt((double)points[k].x / scale,(double)points[k].y / scale);
			pointsf.push_back(pt);
		}
		if (m_bDoClip) {
			// define the destination image
			cv::Mat result = cv::Mat::zeros(round((double)hght / scale), round((double)wdth / scale), image.type());
			// corners of the destination image
			std::vector<cv::Point2f> resultPolygon;
			resultPolygon.push_back(cv::Point2f(0, 0));
			resultPolygon.push_back(cv::Point2f(result.cols - 1, 0));
			resultPolygon.push_back(cv::Point2f(result.cols - 1, result.rows - 1));
			resultPolygon.push_back(cv::Point2f(0, result.rows - 1));


			// get transformation matrix
			cv::Mat transmtx = cv::getPerspectiveTransform(pointsf, resultPolygon);
			// apply perspective transformation
			cv::warpPerspective(image, result, transmtx, result.size());

			if (m_nRotation) {
				cv::Point2f pt(result.cols/2.0, result.rows/2.0);
				cv::Mat r = cv::getRotationMatrix2D(pt, m_nRotation*1.0, 1.0);
				cv::warpAffine(result, image, r, cv::Size(result.cols, result.rows));
			} else {
				image = result;
			}
			ReturnProperty(image);
		}
		
		std::vector<std::vector<cv::Point>> outContours2;
		outContours2.insert(outContours2.begin(), approx2);

		cv::Mat OutputUV(m_imageHeightInPixels / 2, m_imageWidthInPixels / 2,
			CV_8UC2, pDest + m_imageHeightInPixels*lDestStride, lDestStride);
		cv::drawContours(OutputUV, outContours2, -1, cv::Scalar(43, 31), 2);
	}

	// Set the data size on the output buffer.
	hr = pOut->SetCurrentLength(m_cbImageSize);

    return hr;
}

//-------------------------------------------------------------------
// ReturnProperty
// Returns the image data to via PropertySet
//-------------------------------------------------------------------
HRESULT ImageClipping::ReturnProperty(cv::Mat& image)
{
	HRESULT hr = S_OK;

	if (!m_pSetting ||
		!(image.rows > m_imageHeightInPixels / 8 && image.cols > m_imageWidthInPixels / 8)) {
		return (HRESULT)-1;
	}
	std::vector<int> compression_params;
	compression_params.push_back(cv::IMWRITE_JPEG_QUALITY);
	compression_params.push_back(m_nQuality);

	std::vector<uchar> buf;
	bool bCv = cv::imencode(".jpg", image, buf, compression_params);
	if (!bCv) {
		return (HRESULT)-1;
	}

	ComPtr<ABI::Windows::Security::Cryptography::ICryptographicBufferStatics> cryptBuffer;
	HSTRING cryptBufferClassId = Wrappers::HStringReference(RuntimeClass_Windows_Security_Cryptography_CryptographicBuffer).Get();
	hr = ABI::Windows::Foundation::GetActivationFactory(cryptBufferClassId, &cryptBuffer);
	if (!SUCCEEDED(hr)) {
		return hr;
	}

	ABI::Windows::Storage::Streams::IBuffer *buffer;
	hr = cryptBuffer->CreateFromByteArray(buf.size(), buf.data(), &buffer);
	if (!SUCCEEDED(hr)) {
		return hr;
	}

	HSTRING value = NULL;
	hr = cryptBuffer->EncodeToBase64String(buffer, &value);
	if (!SUCCEEDED(hr)) {
		return hr;
	}

	HSTRING key;
	hr = WindowsCreateString(L"{698649BE-8EAE-4551-A4CB-2FA79A4E1E80}", 38, &key);
	if (!SUCCEEDED(hr)) {
		return hr;
	}

	ComPtr<ABI::Windows::Foundation::IPropertyValueStatics> spPropVal;
	HSTRING propValClassId = Wrappers::HStringReference(RuntimeClass_Windows_Foundation_PropertyValue).Get();
	hr = ABI::Windows::Foundation::GetActivationFactory(propValClassId, &spPropVal);
	if (!SUCCEEDED(hr)) {
		return hr;
	}

	ComPtr<IInspectable> spValueInsp;
	boolean replaced;

	hr = spPropVal->CreateString(value, &spValueInsp);
	if (!SUCCEEDED(hr)) {
		return hr;
	}

	if (m_pSetting) {
		hr = m_pSetting->Insert(key, spValueInsp.Get(), &replaced);
	} else {
		return (HRESULT)-1;
	}

	return hr;
}


// Flush the MFT.

HRESULT ImageClipping::OnFlush()
{
    // For this MFT, flushing just means releasing the input sample.
    SafeRelease(&m_pSample);
    return S_OK;
}


// Update the format information. This method is called whenever the
// input type is set.

HRESULT ImageClipping::UpdateFormatInfo()
{
    HRESULT hr = S_OK;

    GUID subtype = GUID_NULL;

    m_imageWidthInPixels = 0;
    m_imageHeightInPixels = 0;
    m_cbImageSize = 0;

    if (m_pInputType != NULL)
    {
        hr = m_pInputType->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (FAILED(hr))
        {
            goto done;
        }
        if (subtype != MFVideoFormat_NV12)
        {
            hr = E_UNEXPECTED;
            goto done;
        }

        hr = MFGetAttributeSize(m_pInputType, MF_MT_FRAME_SIZE, &m_imageWidthInPixels, &m_imageHeightInPixels);
        if (FAILED(hr))
        {
            goto done;
        }

        // Calculate the image size for YUV NV12 image(not including padding)
        m_cbImageSize = (m_imageHeightInPixels + m_imageHeightInPixels/2)*m_imageWidthInPixels;
    }

done:
    return hr;
}


// Get the default stride for a video format.
HRESULT GetDefaultStride(IMFMediaType *pType, LONG *plStride)
{
    LONG lStride = 0;

    // Try to get the default stride from the media type.
    HRESULT hr = pType->GetUINT32(MF_MT_DEFAULT_STRIDE, (UINT32*)&lStride);
    if (FAILED(hr))
    {
        // Attribute not set. Try to calculate the default stride.
        GUID subtype = GUID_NULL;

        UINT32 width = 0;
        UINT32 height = 0;

        // Get the subtype and the image size.
        hr = pType->GetGUID(MF_MT_SUBTYPE, &subtype);
        if (SUCCEEDED(hr))
        {
            hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &width, &height);
        }
        if (SUCCEEDED(hr))
        {
            if (subtype == MFVideoFormat_NV12)
            {
                lStride = width;
            }
            else if (subtype == MFVideoFormat_YUY2 || subtype == MFVideoFormat_UYVY)
            {
                lStride = ((width * 2) + 3) & ~3;
            }
            else
            {
                hr = E_INVALIDARG;
            }
        }

        // Set the attribute for later reference.
        if (SUCCEEDED(hr))
        {
            (void)pType->SetUINT32(MF_MT_DEFAULT_STRIDE, UINT32(lStride));
        }
    }
    if (SUCCEEDED(hr))
    {
        *plStride = lStride;
    }
    return hr;
}

double ImageClipping::angle( cv::Point pt1, cv::Point pt2, cv::Point pt0 ) {
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}
