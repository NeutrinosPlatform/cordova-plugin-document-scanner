#import "Scan.h"

@implementation Scan

- (void)scanDoc:(CDVInvokedUrlCommand*)command
{
    // [sourceType, fileName, quality, returnBase64]
    _commandglo = command;
    NSString* sourceType = [[_commandglo arguments] objectAtIndex:0];
    NSString* fileName = [[_commandglo arguments] objectAtIndex:1];
    NSDecimalNumber *quality = [[_commandglo arguments] objectAtIndex:2];
    id returnBase64 = [[_commandglo arguments] objectAtIndex:3];
    
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
        id returnBase64 = [[_commandglo arguments] objectAtIndex:3];
        
        CGFloat floatQuality = [quality floatValue];
        floatQuality = 1 - (floatQuality - 1)/4; // 1 - 1(quality - 1)/(max - 1)
        NSData *imgData = UIImageJPEGRepresentation(page_image,floatQuality);
        if([returnBase64 boolValue]) {
            CDVPluginResult* result = [CDVPluginResult
                                       resultWithStatus:CDVCommandStatus_OK
                                       messageAsString:[self base64forData:imgData]];
            [self.commandDelegate sendPluginResult:result callbackId:_commandglo.callbackId];
        } else {
            NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
            NSString *documentsPath = [paths objectAtIndex:0]; //Get the docs directory
            NSString *fileName = [[_commandglo arguments] objectAtIndex:1];
            NSString *fileType = @"jpg";
            NSString *completeFileName = [NSString stringWithFormat:@"%@.%@", fileName, fileType];
            
            //Add the file name
            NSString *filePath = [documentsPath stringByAppendingPathComponent:completeFileName];
            
            // Write the file
            [imgData writeToFile:filePath atomically:YES];
            
            // One line code to save to photo library , but no way to obtain the file path
            // UIImageWriteToSavedPhotosAlbum(page_image, nil, nil, nil);
            
            NSString *appendfilestr = @"file://";
            NSString *filePathcomplete = [appendfilestr stringByAppendingString:filePath];
            CDVPluginResult* result = [CDVPluginResult
                                       resultWithStatus:CDVCommandStatus_OK
                                       messageAsString:filePathcomplete];
            [self.commandDelegate sendPluginResult:result callbackId:_commandglo.callbackId];
        }
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

- (NSString*)base64forData:(NSData*)theData {
    const uint8_t* input = (const uint8_t*)[theData bytes];
    NSInteger length = [theData length];
    
    static char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
    
    NSMutableData* data = [NSMutableData dataWithLength:((length + 2) / 3) * 4];
    uint8_t* output = (uint8_t*)data.mutableBytes;
    
    NSInteger i;
    for (i=0; i < length; i += 3) {
        NSInteger value = 0;
        NSInteger j;
        for (j = i; j < (i + 3); j++) {
            value <<= 8;
            
            if (j < length) {
                value |= (0xFF & input[j]);
            }
        }
        
        NSInteger theIndex = (i / 3) * 4;
        output[theIndex + 0] =                    table[(value >> 18) & 0x3F];
        output[theIndex + 1] =                    table[(value >> 12) & 0x3F];
        output[theIndex + 2] = (i + 1) < length ? table[(value >> 6)  & 0x3F] : '=';
        output[theIndex + 3] = (i + 2) < length ? table[(value >> 0)  & 0x3F] : '=';
    }
    
    return [[NSString alloc] initWithData:data encoding:NSASCIIStringEncoding];
}

@end
