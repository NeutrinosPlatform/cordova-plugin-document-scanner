//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
// OpenCVHelper.h

#pragma once
#include <opencv2\core\core.hpp>
#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\video.hpp>

namespace OpenCVBridge
{
    public ref class OpenCVHelper sealed
    {
    public:
        OpenCVHelper(bool bConvertToGrayscale, double alpha);

        // Graphics detection operators
        Platform::Array<int>^ GetPoints(Windows::Graphics::Imaging::SoftwareBitmap^ input);
		Platform::Array<int>^ GetPointsFromMediaFrameReader(Windows::Media::Capture::Frames::MediaFrameReader^ reader);

        void ClipToPoints(
            Windows::Graphics::Imaging::SoftwareBitmap^ input,
            Windows::Graphics::Imaging::SoftwareBitmap^ output,
			const Platform::Array<int>^ quad);

	private:
		// Transformation parameters
		boolean			            m_bDoClip;
		UINT32						m_nQuality;
		boolean			            m_bConvertToGrayscale;
		UINT32						m_nRotation;
		double						m_alpha;

		std::vector<cv::Point>      m_prevPoints;

		double angle( cv::Point pt1, cv::Point pt2, cv::Point pt0 );

        // helper functions for getting a cv::Mat from SoftwareBitmap
        bool GetPointerToPixelData(Windows::Graphics::Imaging::SoftwareBitmap^ bitmap,
            unsigned char** pPixelData, unsigned int* capacity);

        bool TryConvert(Windows::Graphics::Imaging::SoftwareBitmap^ from, cv::Mat& convertedMat);
    };
}
