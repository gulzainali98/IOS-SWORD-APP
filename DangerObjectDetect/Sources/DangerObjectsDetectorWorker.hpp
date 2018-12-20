//
//  DangerObjectsDetectorWorker.hpp
//  SquareDetect
//
//  Created by Dmytro Hrebeniuk on 11/14/18.
//  Copyright © 2018 SquareDetect. All rights reserved.
//

#ifndef DangerObjectsDetectorWorker_hpp
#define DangerObjectsDetectorWorker_hpp

#include <stdio.h>
#include <vector>

namespace cv {
    class Mat;
}

class DangerObjectsDetectorWorker {
    
public:
    cv::Mat detectObjectsInRGBImage(const cv::Mat &inputRGBImage, const cv::Mat &grayImage, int offsetX, int offsetY);
    
private:
    
    cv::Mat createMaskMat(const cv::Mat &image);

    cv::Mat createBlackProcessingMaskMat(const cv::Mat &inputBlack, const cv::Mat &mask, const cv::Mat &equalized, int offsetX, int offsetY);
    
    cv::Mat createWhiteProcessingMaskMat(const cv::Mat &inputGray, const cv::Mat &mask, const cv::Mat &inputRGB, int offsetX, int offsetY);

    cv::Mat createReziedMask(const cv::Mat &sourceMat, const cv::Mat &destinationMat);   
};

#endif /* DengerObjectsDetectorWorker_hpp */
