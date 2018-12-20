//
//  DangerObjectsDetectorWorker.cpp
//  SquareDetect
//
//  Created by Dmytro Hrebeniuk on 11/14/18.
//  Copyright © 2018 SquareDetect. All rights reserved.
//

#include "DangerObjectsDetectorWorker.hpp"
#include <opencv2/cvconfig.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

std::vector<cv::Rect> createBoxes(std::vector<std::vector<cv::Point>> contours) {
    auto boundingBoxes = std::vector<cv::Rect>();
    for(auto contour: contours) {
        cv::Rect rect = cv::boundingRect(contour);
        boundingBoxes.push_back(rect);
    }
    return boundingBoxes;
}

cv::Mat plotBoxes(cv::Mat image, std::vector<cv::Rect> boxes, cv::Scalar color, int width) {
    for(auto box: boxes) {
        cv::rectangle(image, box, color, width);
    }
    return image;
}

std::vector<std::vector<cv::Point>> createContours(const cv::Mat &contourInput, int area, int small, int offsetX, int offsetY) {
    
    cv::Mat image;
    contourInput.copyTo(image);
    
    std::vector<std::vector<cv::Point>> finalContours;
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(image, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    
    for (auto contour : contours) {
        
        std::vector<cv::Point> adjustedContour;
        for (auto point: contour) {
            int pointX = fmax(fmin(point.x + offsetX, image.cols), 0);
            int pointY = fmax(fmin(point.y + offsetY, image.rows), 0);
            adjustedContour.push_back(cv::Point(pointX, pointY));
        }
        
        double contourArea = cv::contourArea(contour);
        if (contourArea > area && !small) {
            finalContours.push_back(adjustedContour);
        }
        else if (contourArea < area && small) {
            finalContours.push_back(adjustedContour);
        }
    }
    
    return finalContours;
}


cv::Mat createHighlightROI(cv::Mat image, cv::Mat mask, cv::Scalar color, int width) {
    cv::Mat result = image.clone();
    auto contours = createContours(mask, 0, false, 0, 0);
    if (contours.size() > 0) {
        auto boxes = createBoxes(contours);
        result = plotBoxes(image, boxes, color, width);
    }
    return result;
}

cv::Mat DangerObjectsDetectorWorker::detectObjectsInRGBImage(const cv::Mat &inputRGBImage, const cv::Mat &grayImage, int offsetX, int offsetY) {
    
    auto colorNumbers = inputRGBImage.step.buf[1];
    cv::Mat rgbImage;
    if (colorNumbers == 1) {
        cv::cvtColor(inputRGBImage, rgbImage, CV_GRAY2BGRA);
    }
    else if (colorNumbers == 3) {
        cv::cvtColor(inputRGBImage, rgbImage, CV_BGR2RGBA);
    }
    else {
        rgbImage = inputRGBImage.clone();
    }

    cv::Mat grayImageMat;
    cv::cvtColor(grayImage, grayImageMat, CV_BGR2GRAY);
    
    cv::GaussianBlur(grayImageMat.clone(), grayImageMat, cv::Size(7, 7), 0, 0);
    
    cv::Mat equalizedMat;
    cv::equalizeHist(grayImageMat.clone(), equalizedMat);
    
    cv::Mat mappedGrayMat;
    cv::applyColorMap(equalizedMat.clone(), mappedGrayMat, cv::COLORMAP_JET);
    
    cv::Mat mask = createMaskMat(mappedGrayMat);
    
    cv::Mat maskBlack = createBlackProcessingMaskMat(mappedGrayMat.clone(), mask, equalizedMat.clone(), offsetX, offsetY);
    cv::Mat eqMask = equalizedMat&mask;
    
    cv::Mat rgbCombined;
    cv::cvtColor(grayImageMat.clone(), rgbCombined, cv::COLOR_GRAY2RGBA);
    
    cv::Mat rgbImageMat;
    cv::cvtColor(equalizedMat.clone(), rgbImageMat, cv::COLOR_GRAY2RGBA);

    cv::Mat whiteMask = createWhiteProcessingMaskMat(eqMask, mask, rgbImageMat, offsetY, offsetX);
    
    whiteMask = createReziedMask(whiteMask, rgbImage);
    
    cv::Mat maskBlackBGRColor[3];
    cv::split(maskBlack, maskBlackBGRColor);
    maskBlack = createReziedMask(maskBlackBGRColor[0], rgbImage);

    cv::Mat colorImage = inputRGBImage.clone();
    for(int y = 0; y < colorImage.rows; y++) {
        for(int x = 0; x < colorImage.cols; x++) {
            auto blackMaskColor = maskBlack.at<uchar>(cv::Point(x, y));
            if (blackMaskColor == 255) {
                colorImage.at<cv::Vec4b>(cv::Point(x, y)) = cv::Vec4b(123, 233, 23, 255);
            }

            auto whiteMaskColor = whiteMask.at<uchar>(cv::Point(x, y));
            if (whiteMaskColor == 255) {
                colorImage.at<cv::Vec4b>(cv::Point(x, y)) = cv::Vec4b(142, 2, 222, 255);
            }
        }
    }
    
    rgbCombined = createHighlightROI(colorImage, whiteMask, CV_RGB(0, 255, 0), 5);
    rgbCombined = createHighlightROI(rgbCombined, maskBlack,  CV_RGB(0, 0, 255), 5);
    
    return rgbCombined;
}

cv::Mat DangerObjectsDetectorWorker::createMaskMat(const cv::Mat &image) {
    cv::Mat compomentsBGRColor[3];
    cv::split(image, compomentsBGRColor);
    
    cv::Mat redImage = compomentsBGRColor[2];
    cv::Mat contourInputMat;
    cv::GaussianBlur(redImage, contourInputMat, cv::Size(7, 7), 0);
    
    for(int y = 0; y<contourInputMat.rows; y++) {
        for(int x = 0; x<contourInputMat.cols; x++) {
            uchar color = contourInputMat.at<uchar>(cv::Point(x,y));
            if (color <= 200) {
                color = 0;
            }
            else {
                color = 255;
            }
            contourInputMat.at<uchar>(cv::Point(x,y)) = color;
        }
    }
    
    cv::morphologyEx(contourInputMat, contourInputMat, cv::MORPH_CLOSE, cv::Mat::ones(3, 3, CV_32F), cv::Point(-1, -1), 2);
    
    auto contours = createContours(contourInputMat, 20000, false, 0, 0);
    
    cv::Mat mask = cv::Mat(contourInputMat.rows, contourInputMat.cols, redImage.type());
    mask.setTo(CV_RGB(0, 0, 0));
    
    cv::fillPoly(mask, contours, CV_RGB(255, 255, 255));
    cv::Mat maskBGRColor[3];
    cv::split(mask, maskBGRColor);
    
    cv::morphologyEx(maskBGRColor[0], mask, cv::MORPH_CLOSE, cv::Mat::ones(13, 13, CV_32F), cv::Point(-1, -1), 5);
    
    // TODO: need implement:  mask = ndimage.binary_fill_holes((mask)).astype("uint8")
    
    for(int y = 0; y< (int)fmin(mask.rows, 50); y++) {
        for(int x = 0; x< mask.cols; x++) {
            mask.at<uchar>(cv::Point(x,y)) = 0;
        }
    }
    
    return mask;
}

cv::Mat DangerObjectsDetectorWorker::createBlackProcessingMaskMat(const cv::Mat &inputBlack, const cv::Mat &mask, const cv::Mat &equalized, int offsetX, int offsetY) {
    cv::Mat seedsImage = equalized.clone();
    
    for(int y = 0; y<seedsImage.rows; y++) {
        for(int x = 0; x<seedsImage.cols; x++) {
            uchar color = seedsImage.at<uchar>(cv::Point(x,y));
            if (color <= 4) {
                color = 0;
            }
            else {
                color = 255;
            }
            
            color=~color;
            
            seedsImage.at<uchar>(cv::Point(x, y)) = color;
        }
    }
    
    cv::Mat multiplyResult = seedsImage&mask;
    auto contours = createContours(multiplyResult, 60, false, offsetX, offsetY);
    
    cv::Mat mask1 = mask.clone();
    mask1.setTo(CV_RGB(0, 0, 0));
    
    for (auto contour: contours) {
        std::vector<cv::Point> hull;
        cv::convexHull(contour, hull, false);
        auto hulls = std::vector<std::vector<cv::Point>>();
        hulls.push_back(hull);
        cv::fillPoly(mask1, hulls, CV_RGB(255, 255, 255));
    }
    
    return mask1;
}

cv::Mat DangerObjectsDetectorWorker::createWhiteProcessingMaskMat(const cv::Mat &inputGray, const cv::Mat &mask, const cv::Mat &inputRGB, int offsetX, int offsetY) {
    
    cv::Mat whiteMat;
    cv::applyColorMap(inputGray, whiteMat, cv::COLORMAP_RAINBOW);
    
    cv::Mat whiteBGRColor[3];
    cv::split(whiteMat, whiteBGRColor);
    
    cv::Mat whiteGrayMat = whiteBGRColor[2];
    for (int y = 0; y < whiteGrayMat.rows; y++) {
        for (int x = 0; x < whiteGrayMat.cols; x++) {
            uchar color = whiteGrayMat.at<uchar>(cv::Point(x, y));
            int value = (color>162 && color<175) ? 256 : color;
            if (value == 256) {
                color = 255;
            }
            else {
                color = 0;
            }
            
            whiteGrayMat.at<uchar>(cv::Point(x, y)) = color;
        }
    }
    
    auto whiteGrayContours = createContours(whiteGrayMat, 2500, true, offsetX, offsetY);
    cv::Mat whiteMask = whiteMat.clone();
    whiteMask.setTo(CV_RGB(0, 0, 0));
    
    for (auto whiteGrayContour: whiteGrayContours) {
        auto contours = std::vector<std::vector<cv::Point>>();
        contours.push_back(whiteGrayContour);
        cv::fillPoly(whiteMask, contours, CV_RGB(255, 255, 255));
    }
    
    cv::Mat whiteBlueBGRColor[3];
    cv::split(whiteMask, whiteBlueBGRColor);
    
    cv::morphologyEx(whiteBlueBGRColor[0], whiteMask, cv::MORPH_OPEN, cv::Mat::ones(3, 3, CV_32F), cv::Point(-1, -1), 5);
    whiteMask = whiteMask&mask;
    
    auto whiteContours = createContours(whiteMask, 0, false, offsetX, offsetY);
    whiteMask = whiteMask.clone();
    whiteMask.setTo(CV_RGB(0, 0, 0));
    for (auto whiteContour: whiteContours) {
        auto moments = cv::moments(whiteContour);
        int cx = moments.m10 / moments.m00;
        int cy = moments.m01 / moments.m00;
        
        auto colors = std::vector<int>();
        for(int y = -5; y <= 5; y++) {
            for(int x = -5; x <= 5; x++) {
                auto color = whiteGrayMat.at<uchar>(cv::Point(cx+x, cy+y));
                colors.push_back(color);
            }
        }
        std::sort (colors.begin(), colors.end());
        if (!colors.empty()) {
            int averageColor = colors[colors.size()/2];
            if (averageColor>225) {
                auto contours = std::vector<std::vector<cv::Point>>();
                contours.push_back(whiteContour);
                cv::fillPoly(whiteMask, contours, CV_RGB(255, 255, 255));
            }
        }
    }
    
    whiteMask = whiteMask&mask;
    
    for(int y = 0; y< (int)(whiteMask.rows*0.3); y++) {
        for(int x = 0; x< whiteMask.cols; x++) {
            whiteMask.at<uchar>(cv::Point(x,y)) = 0;
        }
    }
    
    return whiteMask;
}

cv::Mat DangerObjectsDetectorWorker::createReziedMask(const cv::Mat &sourceMat, const cv::Mat &destinationMat) {
    
    cv::Mat resultMat;
    cv::resize(sourceMat, resultMat, cv::Size(destinationMat.cols, destinationMat.rows));
    
    return resultMat;
}
