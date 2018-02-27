//
//  DcsBuffer.h
//  DCSTest
//
//  Created by dynamsoft on 02/05/2017.
//  Copyright © 2017 dynamsoft. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "DcsException.h"
#import "DcsUIVideoView.h"

/***********************
 * image encode protocol
 * 
 **********************/
@interface DcsEncodeParameter: NSObject

@end
/***********************
 * PNGimage encode protocol interface
 * 
 **********************/
@interface DcsPNGEncodeParameter:DcsEncodeParameter

@end
/***********************
 * JPEG image encode protocol interface
 * @property quality 0.1 to 1.0 ,default is 0.75
 **********************/
@interface DcsJPEGEncodeParameter : DcsEncodeParameter
@property (nonatomic,assign) CGFloat quality;
@end

/************************
 * PDF image encode protocol interface
 * 
 **********************/
@interface DcsPDFEncodeParameter : DcsEncodeParameter
@end

/************************
 * DcsHttpUploadConfig interface
 * 
 **********************/
@interface DcsHttpUploadConfig : NSObject;
	typedef enum {
		DUME_POST=0x0001,
		//DUME_PUT ,//not use
    } DcsUploadMethodEnum;
		
	typedef enum {
		DDFE_BINARY=0x0010,
		DDFE_BASE64} DcsDataFormatEnum;

/**
  specifies the URL for http/https upload.
 */
@property(nonatomic, strong) NSString * url;

/**
 The field name of the upload file when upload through POST.The default value is "name".
 */
@property(nonatomic, strong) NSString * name;

/**
 Specifies the uploading method
 */
@property(nonatomic,assign) DcsUploadMethodEnum  uploadMethod ;

/**
 specifies the data format,either DDFE_BINARY or DDFE_BASE64
 */
@property(nonatomic,assign) DcsDataFormatEnum  dataFormat;

/**
 Specifies the HTML form Field
 */
@property(nonatomic,strong) NSDictionary* formField;

/**
 specifies the HTTP header
 */
@property(nonatomic,strong) NSDictionary *header;

/**
 Specifies the prefix of the file name. The default value is nil
 */
@property(nonatomic, strong) NSString * filePrefix;
@end

/************************
 * DcsHttpDownloadConfig interface
 * 
 **********************/
@interface DcsHttpDownloadConfig : NSObject
/**
  Specifies the URL for the HTTP/HTTPS download.
 */
@property(nonatomic, strong) NSString * url;
/**
  Specifies the HTTP header
 */
@property(nonatomic,strong) NSDictionary *header;
@end


@interface DcsIo : NSObject;


typedef void (^onLoadFailure)(id source, DcsException *exp);
typedef void (^onLoadSuccess)(id source);
typedef BOOL (^onLoadProgress)(NSInteger progress);

typedef BOOL (^onSaveProgress)(NSInteger progress);
typedef void (^onSaveSuccess)(id result);
typedef void (^onSaveFailure)(id result, DcsException *exp);

typedef BOOL (^onUploadProgress)(NSInteger progress);
typedef void (^onUploadFailure)(id userData, DcsException *exception);
typedef void (^onUploadSuccess)(NSData *data);

typedef BOOL (^onDownloadProgress)(NSInteger progress);
typedef void (^onDownloadFailure)(DcsException *exception);
typedef void (^onDownloadSuccess)(NSData* data);

/**
  @since 6.0
  Load the memory data to DcsBuffer in image or document format;
  @param data The data in memroy in jpg,png or pdf formats.
  @param mode The format for loading images. The values represent where it's an image or a document
 */
- (void) loadData:(NSData*) data mode:(DcsModeEnum) mode;
/**
  @since 6.0
  Load images or documents to DcsBuffer.
  @param file The file to be loaded,must be a full path jpg,png or pdf files defined by users.
  @param mode The format for loading images. The values represent where it's an image or a document
 */	
- (void) loadFile:(NSString *)file mode:(DcsModeEnum) mode;
/**
  @since 6.0
  Load images from memory asynchronously and converts them to dcsImage or dcsDocument in DcsBuffer according to the value of mode
  @param data The data in the buffer in jpg,png of pdf formats
  @param mode The format for loading images. The values represent where it's an image or a document.
  @param onSuccess The callback function when this method performs successfully
  @param onFailure The callback function when this method performs unsucessfully.
  @param onProgress The callback function when this method pushes load progress information.
 */
 - (void) loadDataAsync:(NSData *)data mode:(DcsModeEnum) mode 
						successCallback:( onLoadSuccess)onSuccess 
						failureCallback:( onLoadFailure)onFailure 
						progressCallback:( onLoadProgress)onProgress;
/**
  Loads images from files asynchronously and converts them to dcsImage or dcsDocument in DcsBuffer according to the value of mode.
  @param file The file to be loaded. It must be a full path jpg,png or pdf file defined by users.
  @param mode The format for loading images. The values represent where it's an image or a document.
  @param onSuccess  The callback function when this method performs successfully
  @param onFailure  The callback function when this method performs unsucessfully.
  @param onProgress The callback function when this method pushes load progress information.
 */							
- (void)loadFileAsync:(NSString *)file mode:(DcsModeEnum) mode 
						successCallback:( onLoadSuccess)onSuccess 
						failureCallback:( onLoadFailure)onFailure 
						progressCallback:( onLoadProgress)onProgress;
/**
  Saves images in DcsBuffer to the App sandbox directroy synchronously
  @param file To be loaded file ,must be jpg\png or pdf
  @param parameter Its value will be chosen to save image as PNG,JPG or PDF files
 */							
- (void)save:(NSArray *)indices file:(NSString *)filename encodeParameter:( DcsEncodeParameter  *)parameter;
/**
  Saves images in DcsBuffer to the App sandbox directroy aynchronously
  @param indices The indices of images to be saved.
  @param filename To be loaded file ,must be jpg\png or pdf
  @param parameter Its value will be chosen to save image as PNG,JPG or PDF files
  @param onSuccess The callback function when this method performs successfully
  @param onFailure The callback function when this method performs unsucessfully.
  @param onProgress The callback function when this method pushes load progress information.This function will be triggered when each image is processed.
 */	
 - (void)saveAsync: (NSArray *)indices file:(NSString *)filename encodeParameter:(DcsEncodeParameter *)parameter
					successCallback:( onSaveSuccess)onSuccess 
					failureCallback:( onSaveFailure)onFailure 
					progressCallback:( onSaveProgress)onProgress;

/**
  uploads images asynchronously.The selected image data in DcsBuffer will be encoded according to the value of encodeParameter and uploaded to specified http/https server
  @param indices The indices of images to be uploaded.
  @param config Configuration for http/https upload
  @param parameter Its value will be chosen to upload image as PNG,JPG or PDF files
  @param onSuccess The callback function when this method performs successfully
  @param onFailure The callback function when this method performs unsucessfully.
  @param onProgress The callback function when this method pushes load progress information.
 */
 - (void)uploadAsync:(NSArray *)indices uploadConfig:(DcsHttpUploadConfig *)config
                    encodeParameter:(DcsEncodeParameter *)parameter
					successCallback:(onUploadSuccess)onSuccess 
					failureCallback:(onUploadFailure)onFailure 
					progressUpdateCallback:(onUploadProgress)onProgress;

/**
 Dowloads images asynchronously and converts them to dcsImage or dcsDocument in DcsBuffer according to the value of mode.
 @param config Configuration for http/https download
 @param mode Indicates if the downloaded files are images or documents
 @param onSuccess The callback function when this method performs successfully
 @param onFailure The callback function when this method performs unsucessfully.
 @param onProgress The callback function when this method pushes load progress information.
 */
 - (void)downloadAsync:(DcsHttpDownloadConfig *)config
                  mode:(DcsModeEnum) mode
				  successCallback:(onDownloadSuccess)onSuccess
                  failureCallback:(onDownloadFailure)onFailure
                  progressUpdateCallback:(onDownloadProgress)onProgress;

-(instancetype)init:(id)targetClass;

@end


