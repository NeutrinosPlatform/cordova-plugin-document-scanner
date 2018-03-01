#import <Cordova/CDV.h>
@import IRLDocumentScanner;

@interface Scan : CDVPlugin <IRLScannerViewControllerDelegate>
@property CDVInvokedUrlCommand* commandglo;
- (void) scanDoc:(CDVInvokedUrlCommand*)command;

@end
