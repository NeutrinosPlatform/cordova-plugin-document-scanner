//
//  DcsDocument.h
//  DCSTest
//
//  Created by dynamsoft on 05/07/2017.
//  Copyright © 2017 dynamsoft. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DcsImage.h"

@interface DcsDocument : DcsImage

/**
 Creates a DcsDocument object from UIImage
 @param image The image data to create object from
 @return (id) The DcsDocument object
 */
- (id)init:(UIImage *)image;

/**
 Returns the bondary information of the document.
 */
@property (nonatomic, strong, readonly) NSArray *documentBoundary;

/**
 Returns the original image of the document.
 */
@property (nonatomic,strong,readonly) UIImage   *originalImage;



@end
