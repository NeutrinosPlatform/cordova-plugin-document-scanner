//
//  DcsUIImageEditorView.h
//  DCSTest
//
//  Created by dynamsoft on 05/05/2017.
//  Copyright © 2017 dynamsoft. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DcsView.h"
#import "DcsException.h"

#pragma mark --- delegate

@protocol DcsUIImageEditorViewDelegate
- (void)onCancelTapped:(id)sender;
- (void)onOkTapped:(id)sender exception:(DcsException *)exception;
@end

@interface DcsUIImageEditorView : UIView

- (instancetype)initWithFrame:(CGRect)frame withClass:(id)targetClass;

@property (nonatomic, weak) id <DcsUIImageEditorViewDelegate> delegate;

/**
 Gets or sets the view to navigate to when cancel tool item is tapped. The default value is DEV_IMAGEGALLERYVIEW
 */
@property (nonatomic, assign) DcsViewEnum nextViewAfterCancel;

/**
 Gets or sets the view to navigate to when ok tool item is tapped. The default value is DEV_IMAGEGALLERYVIEW
 */
@property (nonatomic, assign) DcsViewEnum nextViewAfterOK;

/**
 Gets or sets whether to show the RotateLeft tool item.The default value is "YES".
 */
@property(nonatomic,assign) BOOL showRotateLeftToolItem;

/**
 Gets or sets whether to show the Rotate right tool item.The default value is "YES".
 */
@property(nonatomic,assign) BOOL showRotateRightToolItem;

/**
 Gets or sets whether to show the flip tool item.The default value is "YES".
 */
@property(nonatomic,assign) BOOL showFlipToolItem;

/**
 Gets or sets whether to show the mirror tool item.The default value is "YES".
 */
@property(nonatomic,assign) BOOL showMirrorToolItem;

/**
 Gets or sets whether to show the brightness tool item.The default value is "YES".
 */
@property(nonatomic,assign) BOOL showBrightnessToolItem;

/**
 Gets or sets whether to show the contrast tool item.The default value is "YES".
 */
@property(nonatomic,assign) BOOL showContrastToolItem;

/**
 Gets or sets whether to show the crop tool item.The default value is "YES".
 */
@property(nonatomic,assign) BOOL showCropToolItem;

/**
 Gets or sets the icon showed on rotate tool item on the image editor view.
 */
@property (nonatomic, strong) UIImage *rotateIcon;

/**
 Gets or sets the icon showed on RotateLeft tool item on the image editor view.
 */
@property (nonatomic, strong) UIImage *rotateLeftIcon;

/**
 Gets or sets the icon showed on RotateRight tool item on the image editor view.
 */
@property (nonatomic, strong) UIImage *rotateRightIcon;

/**
 Gets or sets the icon showed on flip tool item on the image editor view.
 */
@property (nonatomic, strong) UIImage *flipIcon;

/**
 Gets or sets the icon showed on mirror tool item on the image editor view.
 */
@property (nonatomic, strong) UIImage *mirrorIcon;

/**
 Gets or sets the icon showed on crop tool item on the image editor view.
 */
@property (nonatomic, strong) UIImage *cropIcon;

/**
 Gets or sets the icon showed on the Brightness and contrast tool item on the image editor view.
 */
@property (nonatomic, strong) UIImage *brightnessContrastIcon;

/**
 Gets or sets the text showed on the Brightness tool item.The default value is "Brightness".
 */
@property (nonatomic, strong) NSString *brightnessText;


/**
 Gets or sets the text showed on the contrast tool item.The default value is "Contrast".
 */
@property (nonatomic, strong) NSString *contrastText;

/**
 Gets or sets the text showed on the cancel tool item.The default value is "Cancel".
 */
@property (nonatomic, strong) NSString *cancelText;

/**
 Gets or sets the text showed on the OK tool item.The default value is "OK".
 */
@property (nonatomic, strong) NSString *okText;

/**
 Gets or sets the icon showed on the cancel tool item on the image editor view.
 */
@property (nonatomic, strong) UIImage *cropCancelIcon;

/**
 Gets or set the icon showed on the OK tool item on the image editor view.
 */
@property (nonatomic, strong) UIImage *cropOKIcon;

/**
 Adjusts the contrast of an image in the editor.
 @param contrast The amount to increase/decrease the contrast by, ranging from -100 to 100.Negative valuse indicate decreasing contrst while positive values indicate increasing contrast. Each adjustment made to the document is based on the current contrast.
 */
- (void)adjustContrast:(NSInteger)contrast;

/**
 Adjusts the brightness of a document in the editor
 @param brightness The amount to increase/decrease the brightness by, ranging from -100 to 100.Negative valuse indicate decreasing brightness while positive values indicate increasing brightness.Each adjustment made to the document is based on the current brightness.
 
 */
- (void)adjustBrightness:(NSInteger)brightness;

/**
 Rotates the image in the editor by 90 degrees counter-clockwise.
 */
- (void)rotateLeft;

/**
 Rotates the image in the editor by 90 degrees clockwise.
 */
- (void)rotateRight;

/**
 Flips the image in the editor vertically(as opposed to mirror).
 */
- (void)flip;

/**
  Mirrors the image in the editor.
 */
- (void)mirror;

/**
 Cuts an area of the image in the editor.The remaining area is left as white once cut.
 */
- (void)cut:(NSInteger)x1 top:(NSInteger)y1 right:(NSInteger)x2 bottom:(NSInteger)y2;

/**
 Crops an Image in the editor.
 */
- (void)crop:(NSInteger)x1 top:(NSInteger)y1 right:(NSInteger)x2 bottom:(NSInteger)y2;

/**
 Save the changes to the image.
 */
- (void)save;

/**
 Discards the changes of the image.
 */
- (void)discard;





@end
