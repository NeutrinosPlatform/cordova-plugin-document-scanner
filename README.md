# cordova-plugin-document-scanner

> *Note :- Please raise a pull request if there are fixes or enhancements that you want to add!* 

This plugin defines a global `scan` object, which provides an API for scan the document from taking pictures and choosing image from the system's library. 

Although the object is attached to the global scoped `window`, it is not available until after the `deviceready` event.

    document.addEventListener("deviceready", onDeviceReady, false);
    function onDeviceReady() {
        console.log(scan);
    }

## Installation


This requires cordova 7.1.0+ and cordova android 6.4.0+ <br/>
npm link :- https://www.npmjs.com/package/cordova-plugin-document-scanner

 - Add the 3.x.x version of the plugin
```
    cordova plugin add cordova-plugin-document-scanner@3.x.x
```

 - Set these mandatory configuration values in your projects config.xml
```
<preference name="UseSwiftLanguageVersion" value="4.2" />
<preference name="deployment-target" value="10" />
```

 - Go inside your platforms/ios folder then on the cli do
 ```
 pod install
 ```
*Please read issues and fixes section of readme for Ionic & PhoneGap installation*    

### scan.scanDoc(sourceType, successCallback, errorCallback)
Takes a photo using the scan, or retrieves a photo from the device's
image gallery.  The image is passed to the document scanner and the scanned image passed to success callback as the URI for the image file.

The `scan.scanDoc` function opens the device's default scan
application that allows users to snap pictures by default - this behavior occurs,
when `sourceType` equals `1`.
Once the user snaps the photo, the scan application closes and the application is restored.

If `sourceType` is `0`, then a dialog displays
that allows users to select an existing image.

The return value is sent to the [`scanSuccess`](#module_scan.onSuccess) callback function, in
the fileUri formats.

You can do whatever you want with the URI, for
example:

- Render the image in an `<img>` tag.

*Adds file:// in front of the imageuri returned for both android and ios [iOS example imageURI returned :- file:///var/mobile/Containers/Data/Application/8376778A-983B-4FBA-B21C-A4CFDD047AAA/Documents/image.png]*

__Supported Platforms__

- Android
- iOS

**Example**  
```js
scan.scanDoc(sourceType, scanSuccess, scanError);
```

## `scan.scanDoc`

Take a photo and retrieve the image's file location:

    > version 3.x.x example below | Please follow the ios Quirks steps below to get the build to succeed
    ```
    scan.scanDoc(onSuccess, onFail, {sourceType:1}); // sourceType will by default take value 1 if no value is set | 0 for gallery | 1 for camera

    function onSuccess(imageURI) {
        alert(imageURI);
        console.log(imageURI);
        //var image = document.getElementById('myImage');
        //image.src = imageURI; // For iOS, use image.src = imageURI + '?' + Date.now(); to solve issue 10

    }

    function onFail(message) {
        alert('Failed because: ' + message);
    }
    ```
## iOS Quirks

NOTE :- iOS has only document scan via camera for now (Any argument passed will start the camera scan). Document Scan from gallery will be available in future version. Also scanned images aren't saved to the gallery in iOS.

An example file URI obtained from success call back of scanDoc function looks like this  file:///var/mobile/Containers/Data/Application/8376778A-983B-4FBA-B21C-A4CFDD047AAA/Documents/image.png

> for version 3.x.x also keep the following in mind
> - Open xcworkspace file using xcode within the platforms/ios folder. 
> - Select the project, this opens up project settings. Now within General set deployment target in xcode as 10 or above
> - In build settings set swift language version to 4.2
> - In build settings set Objective C bridging header to HelloCordova/Plugins/cordova-plugin-document-scanner/ScanPlugin/Bridging-Header.h where HelloCordova is your project name

## Issues and Fixes

- Error:Execution failed for task ':app:transformNativeLibsWithStripDebugSymbolForDebug' <br/>
    Delete local ndk-bundle folder. Example location :- C:\Users\Administrator\AppData\Local\Android\sdk\ndk-bundle
    
- CropViewController fails in Xcode due to Incompatible Swift Versions <br/>
    Refer issue [13](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/13)
    
- Couldn't find "libopencv_java3.so" [Problem mainly with 64 bit build devices]<br/>
    Refer issue [8](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/8)
    
- iOS scan UI buttons documentation <br/>
    Refer issue [15](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/15)

- Adding plugin in Ionic <br/> 
    Refer 6th response in issue [17](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/17)

- Adding plugin in PhoneGap <br/> 
    Refer entire issue [22](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/22)
    
- iOS: multiple scan does not override the first image <br/> 
    Refer entire issue [10](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/10) 

- Multiple scans don't override the first image | Browser caching issue <br/>
    Refer issue [10](https://github.com/NeutrinosPlatform/cordova-plugin-document-scanner/issues/10) <br/>
    
## Credits / Native library links

Android :- https://github.com/jhansireddy/AndroidScannerDemo <br/>
iOS :- https://github.com/WeTransfer/WeScan

Huge thanks to these authors for making their document scanning native libraries public.

## More about us!

Find out more or contact us directly here :- http://www.neutrinos.co/

Facebook :- https://www.facebook.com/Neutrinos.co/ <br/>
LinkedIn :- https://www.linkedin.com/company/25057297/ <br/>
Twitter :- https://twitter.com/Neutrinosco <br/>
Instagram :- https://www.instagram.com/neutrinos.co/



