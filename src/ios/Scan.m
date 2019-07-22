#import "Scan.h"

@implementation Scan

- (void)scanDoc:(CDVInvokedUrlCommand*)command
{
    _commandglo = command;
    NSString* sourceType = [[_commandglo arguments] objectAtIndex:0];
    NSString* fileName = [[_commandglo arguments] objectAtIndex:1];
    NSDecimalNumber *quality = [[_commandglo arguments] objectAtIndex:2];

    IRLScannerViewController *scanner = [IRLScannerViewController
                                         cameraViewWithDefaultType: IRLScannerViewTypeNormal
                                         defaultDetectorType: IRLScannerDetectorTypePerformance
                                         withDelegate:self];
                                         
    scanner.showControls = YES;
    scanner.showAutoFocusWhiteRectangle = YES;

    [[self topViewController] presentViewController:scanner animated:YES completion:nil];
}

-(void)pageSnapped:(UIImage *)page_image from:(UIViewController *)controller {
    //NSLog(@"%s","Page Snapped....");
    
    [controller dismissViewControllerAnimated:YES completion:^{

        NSDecimalNumber *quality = [[_commandglo arguments] objectAtIndex:2];
        CGFloat floatQuality = [quality floatValue];
        floatQuality = 1 - (floatQuality - 1)/4; // 1 - 1(quality - 1)/(max - 1)
        NSData *imgData = UIImageJPEGRepresentation(page_image,floatQuality);
        NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
        NSString *documentsPath = [paths objectAtIndex:0]; //Get the docs directory
        NSString *fileName = [[_commandglo arguments] objectAtIndex:1];
        NSString *fileType = @"jpg";
        NSString *completeFileName = [NSString stringWithFormat:@"%@.%@", fileName, fileType];
        NSString *filePath = [documentsPath stringByAppendingPathComponent:completeFileName]; //Add the file name
        [imgData writeToFile:filePath atomically:YES]; //Write the file
        
        // One line code to save to photo library , but no way to obtain the file path //UIImageWriteToSavedPhotosAlbum(page_image, nil, nil, nil);

        NSString *appendfilestr = @"file://";
        NSString *filePathcomplete = [appendfilestr stringByAppendingString:filePath];
        CDVPluginResult* result = [CDVPluginResult
                                       resultWithStatus:CDVCommandStatus_OK
                                       messageAsString:filePathcomplete];

        [self.commandDelegate sendPluginResult:result callbackId:_commandglo.callbackId];
    }];
}

-(void)didCancelIRLScannerViewController:(IRLScannerViewController *)cameraView {
    [cameraView dismissViewControllerAnimated:YES completion:nil];
    
    CDVPluginResult* result = [CDVPluginResult
                               resultWithStatus:CDVCommandStatus_ERROR
                               messageAsString:@"Cancelled"];
    [self.commandDelegate sendPluginResult:result callbackId:_commandglo.callbackId];
}

- (UIViewController *)topViewController {
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

@end
