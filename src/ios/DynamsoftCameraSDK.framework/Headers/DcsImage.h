//
//  DcsImage.h
//  DCSTest
//
//  Created by dynamsoft on 05/07/2017.
//  Copyright © 2017 dynamsoft. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DcsImage : NSObject

/**
 Creates a DcsImage object from UIimage.
 @param image The image data to create the object from.
 */
- (id)init:(UIImage *)image;

/**
 Returns corresponding UIImage.
 */
- (UIImage *)uiImage;

/**
 Returns the width of the object in pixel.
 */
- (NSInteger)width;

/**
 Returns the height of the object in pixel.
 */
- (NSInteger)height;

/**
 Creates a DcsImage object from a file.
 @param filename Specifies the file name to create the DcsImage from.
 */
- (id)initWithFile:(NSString *)filename;



@end
