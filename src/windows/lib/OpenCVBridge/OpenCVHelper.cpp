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
// OpenCVHelper.cpp

#include "pch.h"
#include "OpenCVHelper.h"
#include "MemoryBuffer.h"
#include <iostream>
using namespace Microsoft::WRL;

using namespace OpenCVBridge;
using namespace Platform;
using namespace Windows::Graphics::Imaging;
using namespace Windows::Storage::Streams;
using namespace Windows::Foundation;

using namespace cv;

OpenCVHelper::OpenCVHelper(bool bConvertToGrayscale, double alpha)
{
	m_prevPoints.clear();
	m_bDoClip = false;
	m_nQuality = 100;
	m_bConvertToGrayscale = bConvertToGrayscale;
	m_nRotation = 0;
	m_alpha = alpha;
}

bool comp_horz(cv::Point2f a, cv::Point2f b)
{
	return a.x < b.x;
}
bool comp_vert(cv::Point2f a, cv::Point2f b)
{
	return a.y < b.y;
}


Platform::Array<int>^ OpenCVHelper::GetPoints(Windows::Graphics::Imaging::SoftwareBitmap^ input)
{
	Platform::Array<int>^ result;
	int width = input->PixelWidth;
	int height = input->PixelHeight;

    Mat inputMat;
    if (!TryConvert(input, inputMat)) {
        return result;
    }
	double scale = 1.0;
	int max_len = 640;//1280;
	if (width > height && width > max_len) {
        scale = (double)max_len / (double)width;
	} else if (height > max_len) {
        scale = (double)max_len / (double)height;
	}
	if (scale < 1.0) {
		Mat scaled;
		resize(inputMat,scaled,cv::Size(),scale,scale,INTER_NEAREST);
	    width = scaled.cols;
	    height = scaled.rows;
		if (m_alpha != 1.0) {
	        Mat conv;
		    scaled.convertTo(conv, -1, m_alpha, 0);
	        blur(conv, inputMat, cv::Size(5, 5));
		} else {
	        blur(scaled, inputMat, cv::Size(5, 5));
		}
	} else {
		if (m_alpha != 1.0) {
	        Mat conv;
			inputMat.convertTo(conv, -1, m_alpha, 0);
			blur(conv, inputMat, cv::Size(5, 5));
		} else {
			blur(inputMat, inputMat, cv::Size(5, 5));
		}
	}
	int w8 = width / 8;
	int h8= height / 8;
	int w16 = w8 / 2;
	int h16= h8 / 2;

    // blur will enhance edge detection
	//Mat blurred(inputMat);
    //medianBlur(inputMat, blurred, 9);

	Mat gray0(inputMat.size(), CV_8U), gray;

	std::vector<std::vector<cv::Point> > contours;
    //std::vector<std::vector<cv::Point> > squares;

	double smallest_area = (width * height) / 64.0;
	double max_area = 0;

	std::vector<cv::Point> points;
	// find squares in every color plane of the image
    for (int c = 0; c < 3; c++) {
        int ch[] = {c, 0};
        mixChannels(&inputMat, 1, &gray0, 1, ch, 1);

		// try several threshold levels
        //const int threshold_level = 2;
        //for (int l = 0; l < threshold_level; l++) {
            // Use Canny instead of zero threshold level!
            // Canny helps to catch squares with gradient shading
            //if (l == 0) {
                Canny(gray0, gray, 10, 20, 3); //

                // Dilate helps to remove potential holes between edge segments
                dilate(gray, gray, Mat(), cv::Point(-1,-1));
            //} else {
            //    gray = gray0 >= (l+1) * 255 / threshold_level;
            //}

            // Find contours and store them in a list
            findContours(gray, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);

            // Test contours
            std::vector<cv::Point> approx;
            for (size_t i = 0; i < contours.size(); i++) {
                // approximate contour with accuracy proportional
                // to the contour perimeter
				std::vector<cv::Point> hull;
				convexHull(contours[i], hull, true);
				double area = fabs(contourArea(hull));
				if (area > max_area && area > smallest_area) {
					double peri = arcLength(hull, true);
					approxPolyDP(hull, approx, 0.02 * peri, true);
					if (approx.size() == 4) {
						// sort the polygon's corners
						cv::Point2f swap(0, 0);
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
							}
						}
					}
				}
            }
        //}

    }
	if (points.size() == 4) {
		m_prevPoints = points;
	} else if (m_prevPoints.size() == 4) {
		points = m_prevPoints;
		m_prevPoints.clear();
	} else {
		points.clear();
	}
	result = ref new Platform::Array<int>(points.size() * 2);
	for (int i = 0; i < points.size(); i++) {
		result[i*2] = round(points[i].x / scale);
		result[i*2+1] = round(points[i].y / scale);
	}
    return result;
}
Platform::Array<int>^ OpenCVHelper::GetPointsFromMediaFrameReader(Windows::Media::Capture::Frames::MediaFrameReader^ reader)
{
	Platform::Array<int>^ result;
	if (reader) {
		Windows::Media::Capture::Frames::MediaFrameReference^ frame = reader->TryAcquireLatestFrame();
		if (frame && frame->VideoMediaFrame) {
			Windows::Graphics::Imaging::SoftwareBitmap^ raw = frame->VideoMediaFrame->SoftwareBitmap;
			Windows::Graphics::Imaging::SoftwareBitmap^ input = Windows::Graphics::Imaging::SoftwareBitmap::Convert(raw,
				Windows::Graphics::Imaging::BitmapPixelFormat::Bgra8,
                Windows::Graphics::Imaging::BitmapAlphaMode::Premultiplied);
			result = GetPoints(input);
		}
	}
	return result;
}


void OpenCVHelper::ClipToPoints(Windows::Graphics::Imaging::SoftwareBitmap^ input,
    Windows::Graphics::Imaging::SoftwareBitmap^ output, const Platform::Array<int>^ quad)
{
	int inputWidth = input->PixelWidth;
	int inputHeigh = input->PixelHeight;

	int outputWidth = output->PixelWidth;
	int outoutHeigh = output->PixelHeight;

    Mat inputMat, outputMat;
    if (!(quad->Length == 8 && TryConvert(input, inputMat) && TryConvert(output, outputMat))) {
        return;
    }

	cv::Point2f p0, p1, p2, p3;
	p0.x = quad[0];
	p0.y = quad[1];
	p1.x = quad[2];
	p1.y = quad[3];
	p2.x = quad[4];
	p2.y = quad[5];
	p3.x = quad[6];
	p3.y = quad[7];

	std::vector<cv::Point2f> newApprox;
	newApprox.clear();
	newApprox.push_back(p0);
	newApprox.push_back(p1);
	newApprox.push_back(p2);
	newApprox.push_back(p3);

	// define the destination image
	//cv::Mat result = cv::Mat::zeros(outputWidth, outoutHeigh, outputMat.type());
	// corners of the destination image
	std::vector<cv::Point2f> resultPolygon;
	resultPolygon.push_back(cv::Point2f(0, 0));
	resultPolygon.push_back(cv::Point2f(outputMat.cols-1, 0));
	resultPolygon.push_back(cv::Point2f(outputMat.cols-1, outputMat.rows-1));
	resultPolygon.push_back(cv::Point2f(0, outputMat.rows-1));

	// get transformation matrix
	cv::Mat transmtx = getPerspectiveTransform(newApprox, resultPolygon);

	// apply perspective transformation
	if (m_bConvertToGrayscale) {
		Mat grayMat, workMat;
        cvtColor(inputMat, grayMat, CV_BGRA2GRAY);
        cvtColor(grayMat, workMat, CV_GRAY2BGRA);
	    warpPerspective(workMat, outputMat, transmtx, outputMat.size());
	} else {
	    warpPerspective(inputMat, outputMat, transmtx, outputMat.size());
	}
}


bool OpenCVHelper::TryConvert(SoftwareBitmap^ from, Mat& convertedMat)
{
    unsigned char* pPixels = nullptr;
    unsigned int capacity = 0;
    if (!GetPointerToPixelData(from, &pPixels, &capacity))
    {
        return false;
    }

    Mat mat(from->PixelHeight,
        from->PixelWidth,
        CV_8UC4, // assume input SoftwareBitmap is BGRA8 
        (void*)pPixels);

    // shallow copy because we want convertedMat.data = pPixels
    // don't use .copyTo or .clone
    convertedMat = mat;
    return true;
}

bool OpenCVHelper::GetPointerToPixelData(SoftwareBitmap^ bitmap, unsigned char** pPixelData, unsigned int* capacity)
{
    BitmapBuffer^ bmpBuffer = bitmap->LockBuffer(BitmapBufferAccessMode::ReadWrite);
    IMemoryBufferReference^ reference = bmpBuffer->CreateReference();

    ComPtr<IMemoryBufferByteAccess> pBufferByteAccess;
    if ((reinterpret_cast<IInspectable*>(reference)->QueryInterface(IID_PPV_ARGS(&pBufferByteAccess))) != S_OK)
    {
        return false;
    }

    if (pBufferByteAccess->GetBuffer(pPixelData, capacity) != S_OK)
    {
        return false;
    }
    return true;
}

double OpenCVHelper::angle( cv::Point pt1, cv::Point pt2, cv::Point pt0 ) {
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2)/sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}


