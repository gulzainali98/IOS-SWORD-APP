//
//  DangerObjectsDetector.m
//  SquareDetect
//
//  Created by Dmytro Hrebeniuk on 11/14/18.
//  Copyright © 2018 SquareDetect. All rights reserved.
//

#import "DangerObjectsDetector.h"
#include <opencv2/imgcodecs/ios.h>
#include "DangerObjectsDetectorWorker.hpp"

@implementation DangerObjectsDetector

- (UIImage *)detectObjectsInRGBImage:(UIImage *)rgbImage inGrayImage:(UIImage *)grayImage offsetSize:(CGSize)offsetSize {
    cv::Mat rgbMatImage;
    cv::Mat grayMatImage;
    
    UIImageToMat(rgbImage, rgbMatImage);
    UIImageToMat(grayImage, grayMatImage);
    
    DangerObjectsDetectorWorker *dangerObjectsDetectorWorker = new DangerObjectsDetectorWorker();
    cv::Mat result = dangerObjectsDetectorWorker->detectObjectsInRGBImage(rgbMatImage, grayMatImage, offsetSize.width, offsetSize.height);
    UIImage *resultImage = MatToUIImage(result);
    
    delete dangerObjectsDetectorWorker;
    
    return resultImage;
}

@end
