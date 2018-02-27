//
//  DcsUIVideoView.h
//  DCSTest
//
//  Created by dynamsoft on 04/05/2017.
//  Copyright © 2017 dynamsoft. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DcsException.h"
#import "DcsView.h"
#import "DcsImage.h"
#import "DcsDocument.h"

#pragma mark -- Delegate

@protocol DcsUIVideoViewDelegate


/**
 * @since 6.0
 * onDocumentDetected
 * This delegate is fired when a doucument is detected
 *
 * @param sender   The DcsUIVideoView object
 * @param document The document that been dectected.
 */
- (void)onDocumentDetected:(id)sender document:(DcsDocument *)document;

/**
 * @since 6.0
 * onCancelTapped
 * This delegate is fired when the cancel tool item is clicked
 *
 * @param sender The DcsUIVideoView object 
 */
- (void)onCancelTapped:(id)sender;

/**
 * @since 6.0
 * The delegate is fired whe the capture tool item is clicked.
 *
 * @param sender The DcsUIVideoView object 
 */
- (void)onCaptureTapped:(id)sender;

/**
 * @since 6.0
 * The delegate is fired before an image or a document is captrued
 *
 * @param sender The DcsUIVideoView object 
 */
- (BOOL)onPreCapture:(id)sender;

/**
 * @since 6.0
 * The delegate is fired after an image or a document is captured.
 *
 * @param sender The DcsUIVideoView object 
 */
- (void)onPostCapture:(id)sender;

/**
 * @since 6.0
 * The delegate is fired when it fails to capture an image or a docment
 *
 * @param sender The DcsUIVideoView object 
 */
- (void)onCaptureFailure:(id)sender exception:(DcsException*)exception;
@end


@interface DcsUIVideoView : UIView

- (instancetype)initWithFrame:(CGRect)frame withClass:(id)targetClass;

#pragma mark -- API
//Video mode
typedef enum{
    DME_IMAGE = 0x0001,//Image mode
    DME_DOCUMENT ,//document mode
}DcsModeEnum;

//camera Position
typedef enum{
    DCPE_BACK = 0x0010,
    DCPE_FRONT ,
}DcsCameraPositionEnum;

//flash mode
typedef enum {
    DFME_ON = 0x0100,
    DFME_OFF ,
    DFME_AUTO ,
    DFME_TORCH 
}DcsFlashModeEnum;

//delegate
@property (nonatomic, weak) id <DcsUIVideoViewDelegate> delegate;


/**
 Gets or sets the view to navigate to when cancel tool item is tapped.The default value is DEV_IMAGEGALLERYVIEW
 */
@property (nonatomic, assign) DcsViewEnum nextViewAfterCancel;

/**
 Gets or sets the view to navigate to when capture tool item is tapped.The default value is DEV_VIDEOVIEW
 */
@property (nonatomic, assign) DcsViewEnum nextViewAfterCapture;

/**
  Gets or sets the view mode.The default is DME_DOCUMENT.
 */
@property (nonatomic, assign) DcsModeEnum mode;

/**
 Gets or sets the view mode.The default is DCPE_BACK.
 */
@property (nonatomic, assign) DcsCameraPositionEnum cameraPosition;

/**
 Gets or sets the flash mode.The default is DFME_OFF.
 */
@property(nonatomic,assign) DcsFlashModeEnum flashMode;


/**
  Gets or sets the document border color.The default is #5eb7e4
 */
@property (nonatomic,strong) UIColor *documentBoundaryColor;

/**
 Gets or sets the document border thickness.The default is 6px
 */
@property (nonatomic,assign) NSInteger documentBoundaryThickness;

/**
 Gets or sets whether to display the flash tool item.The default is YES.
 */
@property (nonatomic, assign) BOOL showFlashToolItem;

/**
 Gets or sets whether to display the capture tool item.The default is YES.
 */
@property (nonatomic,assign) BOOL showCaptureToolItem;

/**
  Gets or sets whether to display the cancel tool item.The default is YES.
 */
@property (nonatomic,assign) BOOL showCancelToolItem;

/**
 Gets or sets the text showed on the cancel tool item on the DcsUIVideoView. The default is "Cancel".
 */
@property (nonatomic,strong) NSString *cancelText;
/**
 Gets or sets the text showed on the flash tool item on the DcsUIVideoView,when the flash is on.The default is "On".
 */
@property (nonatomic,strong) NSString *flashOnText;

/**
 Gets or sets the text showed on the flash tool item on the DcsUIVideoView,when the flash is off.The default is "Off".
 */
@property (nonatomic,strong) NSString *flashOffText;

/**
 Gets or sets the text showed on the flash tool item on the DcsUIVideoView,when the flash is in auto state.The default is "Auto".
 */
@property (nonatomic,strong) NSString *flashAutoText;

/**
 Gets or sets the text showed on the flash tool item on the DcsUIVideoView,when the flash is in torch state.The default is "Torch".
 */
@property (nonatomic,strong) NSString *flashTorchText;

/**
 Gets or sets the icon showed on flash tool item on the DcsUIVideoView when flash is in on state.
 */
@property (nonatomic, strong)  UIImage *flashOnIcon;

/**
 Gets or sets the icon showed on flash tool item on the DcsUIVideoView when flash is in off state.
 */
@property (nonatomic, strong)  UIImage *flashOffIcon;

/**
 Gets or sets the icon showed on flash tool item on the DcsUIVideoView when flash is in auto state.
 */
@property (nonatomic, strong)  UIImage *flashAutoIcon;

/**
 Gets or sets the icon showed on flash tool item on the DcsUIVideoView when flash is in torch state.
 */
@property (nonatomic, strong)  UIImage *flashTorchIcon;


/**
 Gets or sets the icon showed on capture tool item on the DcsUIVideoView.
 */
@property (nonatomic, strong)  UIImage *captureIcon;

/**
 Opens the video.
 If the capture mode is DME_DOCUMENT,the document border will be detected in real time and displaye on the video stream.If the mobile camra is not authorized for use,the exception DcsCameraNotAuthorizedException will be thrown.
 */
- (void)preview;

/**
 Stops the video
 If the mobile camra is not authorized for use,the exception DcsCameraNotAuthorizedException will be thrown.
 *
 */
- (void)stopPreview;

/**
 captures an image
 Make sure the current camera is open before calling this method.You can open the camera via 'preview' method.
 */
- (void)captureImage;

/**
 Captures a document
 The function is executed asynchronously.It will first trigger the event 'onPreCapture',then the capture operation,and finally,trigger the event onPostCapture.Make sure the current camera is open before calling this method.You can open the camera via 'preview' method.
 */
- (void)captureDocument;


/**
 Gets or sets whether to disable document capture function when no document is detected.The default value is YES.
 */
@property (nonatomic, assign) BOOL ifAllowDocumentCaptureWhenNotDetected;




@end
