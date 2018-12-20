//
//  DangerObjectsDetector.h
//  SquareDetect
//
//  Created by Dmytro Hrebeniuk on 11/14/18.
//  Copyright © 2018 SquareDetect. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <CoreGraphics/CoreGraphics.h>

@interface DangerObjectsDetector : NSObject

- (UIImage *)detectObjectsInRGBImage:(UIImage *)rgbImage inGrayImage:(UIImage *)grayImage offsetSize:(CGSize)offsetSize;

@end
