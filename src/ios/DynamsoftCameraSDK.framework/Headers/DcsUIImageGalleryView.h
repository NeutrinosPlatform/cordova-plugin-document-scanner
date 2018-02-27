//
//  DcsUIImageView.h
//  DCSTest
//
//  Created by dynamsoft on 10/05/2017.
//  Copyright © 2017 dynamsoft. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import "DcsBuffer.h"
#import "DcsView.h"

#pragma mark - delegate
@protocol DcsUIImageGalleryViewDelegate
/**
 Single click event.It is fired when the image with a specified index in the image gallery view is clicked.
 @param sender The current UIImageGalleryView object that triggers the event
 @param index The index of an image
 */
- (void)onSingleTap: (id)sender index:(NSInteger)index;
/**
 The event is fired when the  indices of the selected images or documents in DcsBuffer are changed.It returns the indices of the selected images or documents in DcsBuffer after the change.
 @param sender The current UIImageGalleryView object that triggers the event
 @param indices The indices of selected images or documents
 */
- (void)onSelectChanged:(id)sender selectedIndices:(NSArray *)indices;
/**
 Long press event. It is fired under the view mode DIVME_MULTIPLE when an image with a specified index in the image gallery view is long pressed.
 @param sender The current UIImageGalleryView object that triggers the event
 @param index The index of an image
 */
- (void)onLongPress: (id)sender index: (NSInteger)index;

@end

@interface DcsUIImageGalleryView : UIView
/**
  DcsUIImageGalleryView visible mode:multiple mode or Single mode
 */
typedef enum {
    DIVME_MULTIPLE = 0x0001,
    DIVME_SINGLE
}DcsImageGalleryViewModeEnum;

//delegate
@property (nonatomic, weak) id <DcsUIImageGalleryViewDelegate> delegate;

/**
 Gets or sets the view mode. The default is DIVME_MULTIPLE.
 */
@property(nonatomic,assign) DcsImageGalleryViewModeEnum imageGalleryViewmode;

/**
 Enables the manual sorting function.
 The manual sorting function is only avaliable when the view mode is DIVME_MULTIPLE.If the view mode is DIVME_SINGLE,when it is changed to DIVME_MULTIPLE the soring function is activated.
 */
- (void)enterManualSortMode;

/**
 Enter the normal mode
 */
- (void)enterNormalMode;

/**
 Enables the images selection function
 The image selection function is only avaliable when the view mode is DIVME_MULTIPLE.If the view mode is DIVME_SINGLE,when it is changed to DIVME_MULTIPLE the soring function is activated.
 */
- (void)enterSelectMode;


/**
 Gets or sets the indices of the currently selected images or documents
 */
@property(nonatomic, strong) NSArray *selectedIndices;

- (instancetype)initWithFrame:(CGRect)frame withClass:(id)targetClass;
@end
