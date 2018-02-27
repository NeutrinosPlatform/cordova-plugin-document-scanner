//
//  DcsUIDocumentEditorView.h
//  DCSTest
//
//  Created by dynamsoft on 05/05/2017.
//  Copyright © 2017 dynamsoft. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DcsBuffer.h"
#import "DcsView.h"
#import "DcsException.h"

@protocol DcsUIDocumentEditorViewDelegate

- (void)onCancelTapped:(id)sender;

- (void)onOkTapped:(id)sender exception:(DcsException *)exception;

@end

@interface DcsUIDocumentEditorView : UIView

- (instancetype)initWithFrame:(CGRect)frame withClass:(id)targetClass;

@property (nonatomic, weak) id <DcsUIDocumentEditorViewDelegate> delegate;


/**
  Gets or sets the view to navigate to when cancel tool item is tapped. The default value
  is DEV_IMAGEGALLERYVIEW
 */
@property (nonatomic, assign) DcsViewEnum nextViewAfterCancel;

/**
  Gets or sets the view to navigate to when ok tool item is tapped. The default value
  is DEV_IMAGEGALLERYVIEW
 */
@property (nonatomic, assign) DcsViewEnum nextViewAfterOK;


/**
  Gets or sets teh document border color.The default is #5eb7e4
 */
@property(nonatomic, strong) UIColor *documentBoundaryColor;

/**
  Gets or sets the document border thickness. The default is 6px.
 */
@property(nonatomic, assign) NSInteger documentBoundaryThickness;

/**
  Gets or sets the text showed on Brightness tool item on the document editor viewer. The default value is “Brightness”.
 */
@property(nonatomic,strong) NSString * brightnessText;

/**
 Gets or sets the text showed on Contrast tool item on the document editor viewer. The default value is “Contrast”.
 */
@property(nonatomic,strong) NSString * contrastText;

/**
 Gets or sets the text showed on ColorMode tool item on the document editor viewer. The default value is “Color”.
 */
@property(nonatomic,strong) NSString * colorText;

/**
 Gets or sets the text showed on GreyMode tool item on the document editor viewer. The default value is “Grey”.
 */
@property(nonatomic,strong) NSString * greyText;

/**
  Gets or set the text showed on B/W Mode tool item on the document editor viewer. The default value is “B/W”.
 */
@property(nonatomic,strong) NSString * blackWhiteText;

/**
  Gets or set the text showed on Cancel tool item on the document editor viewer. The default value is “Cancel”.
 */
@property(nonatomic,strong) NSString * cancelText;

/**
  Gets or set the text showed on OK tool item on the document editor viewer. The default value is “OK”.
 */
@property(nonatomic,strong) NSString * okText;

/**
  Returns or set whether to show the RotateLeft tool item.
 */
@property(nonatomic,assign) BOOL showRotateLeftToolItem;

/**
  Returns or set whether to show the RotateRight tool item.
 */
@property(nonatomic,assign) BOOL showRotateRightToolItem;

/**
  Returns or set whether to show the Brightness tool item.
 */
@property(nonatomic,assign) BOOL showBrightnessToolItem;

/**
  Returns or set whether to show the Contrast tool item.
 */
@property(nonatomic,assign) BOOL showContrastToolItem;

/**
  Returns or set whether to show the ImageMode tool item.
 */
@property(nonatomic,assign) BOOL showImageModeToolItem;

/**
  Returns or set the icon showed on RotateLeft tool item on the document editor viewer.
 */
@property(nonatomic, strong)  UIImage *rotateLeftIcon;

/**
  Returns or set the icon showed on RotateRight tool item on the document editor viewer.
 */
@property(nonatomic, strong)  UIImage *rotateRightIcon;

/**
  Returns or set the icon showed on BrightnessContrast tool item on the document editor viewer.
 */
@property(nonatomic, strong)  UIImage *brightnessContrastIcon;

/**
  Returns or set the icon showed on ImageMode tool item on the document editor viewer.
 */
@property(nonatomic, strong)  UIImage *imageModeIcon;

/**
  Returns or set the icon showed on ColorMode tool item on the document editor viewer.
 */
@property(nonatomic, strong)  UIImage *colorModeIcon;

/**
  Returns or set the icon showed on GreyMode tool item on the document editor viewer.
 */
@property(nonatomic, strong)  UIImage *greyModeIcon;

/**
  Returns or set the icon showed on B/W Mode tool item on the document editor viewer.
 */
@property(nonatomic, strong)  UIImage *blackWhiteIcon;

/**
 Adjusts the contrast of a document in the editor
 @param contrast The amount to increase/decrease the contrast by, ranging from -100 to 100.Negative valuse indicate decreasing contrst while positive values indicate increasing contrast. Each adjustment made to the document is based on the current contrast.

 */
- (void)adjustContrast:(NSInteger)contrast;

/**
 Adjusts the brightness of a document in the editor
 @param brightness The amount to increase/decrease the brightness by, ranging from -100 to 100.Negative valuse indicate decreasing brightness while positive values indicate increasing brightness.Each adjustment made to the document is based on the current brightness.
 
 */
- (void)adjustBrightness:(NSInteger)brightness;

/**
  Rotates the document in the editor by 90 degrees counter-clockwise
 */
- (void)rotateLeft;

/**
 Rotates the document in the editor by 90 degrees clockwise
 */
- (void)rotateRight;

/**
  Converts to a 24-bit RGB document
 */
- (void)toColor;

/**
 Converts to a greyscale document
 */
- (void)toGrey;

/**
 Converts to a black and white document
 */
- (void)toBlackWhite;

/**
  Saves the changes made to the document
 */
- (void)save;

/**
 Discards the changes made to the document
 */
- (void)discard;

@end
