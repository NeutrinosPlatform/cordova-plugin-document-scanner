#import "Scan.h"
//#import "Scanui.h"
//#import "AssetsLibrary/AssetsLibrary.h";


@implementation Scan

- (void)scanDoc:(CDVInvokedUrlCommand*)command
{
    _commandglo = command;
    NSString* sourceType = [[_commandglo arguments] objectAtIndex:0];
    NSString* fileName = [[_commandglo arguments] objectAtIndex:1];
    
    //NSLog(@"%s","I AM HERE AT THE STAAAAAART!!!");
    //NSLog(@"%@",name);
    //IRLScannerViewController *scanner = [IRLScannerViewController standardCameraViewWithDelegate:self];

    // [IRLScannerViewController attemptRotationToDeviceOrientation];

    IRLScannerViewController *scanner = [IRLScannerViewController
                                         cameraViewWithDefaultType: IRLScannerViewTypeNormal
                                         defaultDetectorType: IRLScannerDetectorTypePerformance
                                         withDelegate:self];
    //[scanner preferredInterfaceOrientationForPresentatio];
    scanner.showControls = YES;
    scanner.showAutoFocusWhiteRectangle = YES;
    
    //[TOCropViewController attemptRotationToDeviceOrientation];
    //[scanner viewDidAppear:<#(BOOL)#>];
    //[[self topViewController] attemptRotationToDeviceOrientation];
    
    [[self topViewController] presentViewController:scanner animated:YES completion:nil];
    
    //    Scanui* scanui = [[Scanui alloc] init];
    //    [scanui viewDidLoad];
    //    UIView* cview = [self webView];
    //
    //    [scanui scanirlui:0 and:cview] ;
    
    
    //[scanui scanirlui:0] ;
    //   NSString* name = [[command arguments] objectAtIndex:0];
    //    NSString* msg = [NSString stringWithFormat: @"Hello, %@", name];
    //
    //    CDVPluginResult* result = [CDVPluginResult
    //                               resultWithStatus:CDVCommandStatus_OK
    //                               messageAsString:msg];
    //
    //    [self.commandDelegate sendPluginResult:result callbackId:command.callbackId];
    
    //    IRLScannerViewController *scanner = [IRLScannerViewController standardCameraViewWithDelegate:self];
    //    scanner.showControls = YES;
    //    scanner.showAutoFocusWhiteRectangle = YES;
    //    [self presentViewController:scanner animated:YES completion:nil];
    
}



-(void)pageSnapped:(UIImage *)page_image from:(UIViewController *)controller {
    //NSLog(@"%s","Page Snapped....");
    
    [controller dismissViewControllerAnimated:YES completion:^{
        
        // [self.scannedImage setImage:page_image];
        
        NSData *pngData = UIImagePNGRepresentation(page_image);
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsPath = [paths objectAtIndex:0]; //Get the docs directory
        NSString *fileName = [[_commandglo arguments] objectAtIndex:1];
        NSString *fileType = @"png";
        NSString *completeFileName = [NSString stringWithFormat:@"%@.%@", fileName, fileType];
        NSString *filePath = [documentsPath stringByAppendingPathComponent:completeFileName]; //Add the file name
        [pngData writeToFile:filePath atomically:YES]; //Write the file
        //NSLog(@"%s","Page Snapped.222...");
        
        ////// One line code to save to photo library , but no way to obtain the file path //UIImageWriteToSavedPhotosAlbum(page_image, nil, nil, nil);
        
        
        
        //        UIImage *viewImage = page_image; // --- mine was made from drawing context
        //        ALAssetsLibrary *library = [[ALAssetsLibrary alloc] init];
        //        // Request to save the image to camera roll
        //        [library writeImageToSavedPhotosAlbum:[viewImage CGImage] orientation:(ALAssetOrientation)[viewImage imageOrientation] completionBlock:^(NSURL *assetURL, NSError *error){
        //            if (error) {
        //                NSLog(@"error");
        //            } else {
        //                NSLog(@"url %@", assetURL);
        //            }
        //        }];
        //        [library release];




        // NSString* name = [[_commandglo arguments] objectAtIndex:0];
        // NSString* msg = [NSString stringWithFormat: @"Hello, %@", name];
        //
        NSString *appendfilestr = @"file://";
        //NSString *string2 = @" a test.";
        NSString *filePathcomplete = [appendfilestr stringByAppendingString:filePath];
        // string3 is now @"This is a test."  string1 and string2 are unchanged.
        
        
            CDVPluginResult* result = [CDVPluginResult
                                       resultWithStatus:CDVCommandStatus_OK
                                       messageAsString:filePathcomplete];
        //
            [self.commandDelegate sendPluginResult:result callbackId:_commandglo.callbackId];
        
        
        //////////////////
    }];
}

-(void)didCancelIRLScannerViewController:(IRLScannerViewController *)cameraView {
    [cameraView dismissViewControllerAnimated:YES completion:nil];
    
    CDVPluginResult* result = [CDVPluginResult
                               resultWithStatus:CDVCommandStatus_ERROR
                               messageAsString:@"Cancelled"];
    //
    [self.commandDelegate sendPluginResult:result callbackId:_commandglo.callbackId];
    //CDVCommandStatus_ERROR
}

- (UIViewController *)topViewController{
    return [self topViewController:[UIApplication sharedApplication].keyWindow.rootViewController];
}

- (UIViewController *)topViewController:(UIViewController *)rootViewController
{
    if (rootViewController.presentedViewController == nil) {
        return rootViewController;
    }
    
    if ([rootViewController.presentedViewController isKindOfClass:[UINavigationController class]]) {
        UINavigationController *navigationController = (UINavigationController *)rootViewController.presentedViewController;
        UIViewController *lastViewController = [[navigationController viewControllers] lastObject];
        return [self topViewController:lastViewController];
    }
    
    UIViewController *presentedViewController = (UIViewController *)rootViewController.presentedViewController;
    return [self topViewController:presentedViewController];
}

//-(void)viewDidAppear:(BOOL)animated{
//
//    [[UIDevice currentDevice] setValue:
//     [NSNumber numberWithInteger: UIInterfaceOrientationLandscapeLeft]
//                                forKey:@"orientation"];
//    
//}

@end
