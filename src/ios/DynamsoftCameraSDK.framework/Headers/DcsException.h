//
//  DcsBuffer.h
//  DCSTest
//
//  Created by dynamsoft on 02/05/2017.
//  Copyright © 2017 dynamsoft. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

@interface DcsException : NSException
-(instancetype) init: (NSString*) reason;
@end
/**
 The exception class indicate the value of the parameter is out of range.
*/
@interface DcsValueOutOfRangeException : DcsException
/**
 Create a DcsValueOutOfRangeException object
 @param pName The parameter name which's value out of range
 */
-(instancetype) init: (NSString*) pName;
/**
 Throw a DcsValueOutOfRangeException object
 @param pName The parameter name which's value out of range
 */
-(void) throwDcsException : (NSString*) pName;
@end

/**
  The exception class indicate the value of the parameter is invalid.
*/
@interface DcsValueNotValidException : DcsException
-(instancetype) init: (NSString*) pName;
-(void) throwDcsException : (NSString*) pName;
@end
/**
 The exception class indicate the document is not ready for editing.
*/
@interface DcsDocumentNotReadyException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
The exception class indicate the file cannot be found..
*/
@interface DcsFileNotFoundException : DcsException
-(instancetype) init:(NSString*)pFileName;
-(void) throwDcsException:(NSString*)pFileName;
@end
/**
The exception class indicate the operation is only supported under document mode.
*/
@interface DcsDocumentOnlyException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
The exception class indicate the operation is only supported under image  mode.
*/
@interface DcsImageOnlyException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate the multiple selection can only be enabled in select mode.
 */
@interface DcsSelectModeNotEnabledException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate the no document is detected.
 */
@interface DcsDocumentNotDetectedException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end

@interface DcsLicenseException : DcsException
@end
/**
 The exception class indicate the type  is invalid.
 */
@interface DcsTypeNotValidException : DcsException
-(instancetype) init: (NSString*) pName;
-(void) throwDcsException: (NSString*) pName;
@end

/**
 The exception class indicate  an error occurred while reading a file.
 */
@interface DcsFileReadException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate that the file path does not exist or you do not have permission to access it.
 */
@interface DcsFilePathInvalidException : DcsException
-(instancetype) init:(NSString*)pFileName;
-(void) throwDcsException:(NSString*)pFileName;
@end
/**
 The exception class indicate that there is not enough storage space available to complete this operation.
 */
@interface DcsNotEnoughSpaceException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate that the server returned the error code [HTTP_status_codes]
 */
@interface DcsHttpErrorException : DcsException
-(instancetype) init: (NSInteger) HTTP_status_codes;
-(void) throwDcsException: (NSInteger) HTTP_status_codes;
@end
/**
 The exception class indicate that the user are trying to load an unsupported data.
 */
@interface DcsDataFormatInvalidException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
The exception class indicate that user cancelled the operation.
 */
@interface DcsOperationCancelledException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate the network unconnected.
 */
@interface DcsNetworkUnconnectedException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
The exception class indicate the camera not be authorized
 */
@interface DcsCameraNotAuthorizedException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate the object does not exist.
 */
@interface DcsObjectNotExistException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end

/**
 The exception class indicate the operation out of expected sequence.
 */
@interface DcsOperationSequenceException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate the access to the cached license invalid.
 */
@interface DcsLicenseInvalidException : DcsLicenseException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate the access to the cached license expired.
 */
@interface DcsLicenseExpiredException : DcsLicenseException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate the camre not open
 */
@interface DcsCameraNotOpenException : DcsLicenseException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
The exception class indicate the number of devices has exceeded the limit of the license.
 */
@interface DcsLicenseDevicesNumberExceededException : DcsLicenseException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate the license verification failed.
 */
@interface DcsLicenseVerificationFailedException : DcsLicenseException
-(instancetype) init;
-(void) throwDcsException;
@end
/**
 The exception class indicate the delegate undefine
 */
@interface DcsDelegateUndefineException : DcsException
-(instancetype) init: (NSString*) pName;
-(void) throwDcsException: (NSString*) pName;
@end

@interface DcsNullException : DcsException
-(instancetype) init;
-(void) throwDcsException;
@end
