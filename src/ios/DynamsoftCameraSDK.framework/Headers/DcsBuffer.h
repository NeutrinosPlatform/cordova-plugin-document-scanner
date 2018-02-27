//
//  DcsBuffer.h
//  DCSTest
//
//  Created by dynamsoft on 02/05/2017.
//  Copyright © 2017 dynamsoft. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "DcsImage.h"
#import "DcsDocument.h"

@interface DcsBuffer : NSObject

#pragma mark - APIs

/**
 Returns or sets the index of current operated data in DcsBuffer
 
 */
@property(nonatomic,assign)NSInteger currentIndex;

/**
 Appends images to DcsBuffer
 @param image The image to be added into the buffer
 */
- (void)appendImage:(DcsImage *)image;

/**
 Appends documents to DcsBuffer
 @param document The document to be added into the buffer
 */
- (void)appendDocument:(DcsDocument *)document;

/**
 Gets the image or document with the specified index in  the buffer
 @param index Specifices the index of an image or a document.The index is 0-based
 @return (id) The image or document with the spcified index in the buffer
 */
- (id)get:( NSInteger )index;

/**
 Replaces the data at specified index of buffer with the specified DcsImage
 @param image The image to be added to the buffer
 @param index The specified index of the image to be replaced
 */
- (void)replaceWithImage:(DcsImage *)image index:(NSInteger)index;

/**
 Replaces the data at specified index of buffer with the specified DcsDocument
 @param document The document to be added to the buffer
 @param index The specified index of the document to be replaced
 */
- (void)replaceWithDocument:(DcsDocument *)document index:(NSInteger)index;


/**
 Switched the places for two images or documents
 @param firstIndex The index of the first object to be switched
 @param secondIndex The index of the seconed object to be switched
 */
- (void)swap:(NSInteger)firstIndex second:(NSInteger)secondIndex;

/**
 Delete the image or document with the specified index
 @param index The index of the image to be deleted
 */
- (void)delete:( NSInteger)index;

/**
 Saves all the images and documents in DcsBuffer to a folder named dirName
 @param dirName The name of the folder to save files
 */
- (void)save:(NSString *)dirName;

/**
 Loads all the images and documents in the dirName folder to DcsBuffer
 @param dirName The name of the folder to load files from
 */
- (void)load:(NSString *)dirName;

/**
 Returns the total number of images and documents in DcsBuffer
 @return (NSInteger) The total number.
 */
- (NSInteger)count;

/**
  Inserts an image into DcsBuffer at the index
  @param image The image data to be inserted
  @param index The index to insert the image
 */
- (void)insertImage:(DcsImage *)image index:(NSInteger)index;

/**
  Inserts a document into DcsBuffer at the index 
  @param document The image data to be inserted
  @param index The index to insert the image
 */
- (void)insertDocument:(DcsDocument *)document index:(NSInteger)index;


- (instancetype)initWithClass:(id)targetClass;



@end
