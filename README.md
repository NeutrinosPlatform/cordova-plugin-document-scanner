# cordova-plugin-document-scanner

This plugin defines a global `scan` object, which provides an API for scan the document from taking pictures and choosing image from the system's library. 

Although the object is attached to the global scoped `window`, it is not available until after the `deviceready` event.

    document.addEventListener("deviceready", onDeviceReady, false);
    function onDeviceReady() {
        console.log(scan);
    }

## Installation


This requires cordova 5.0+

    cordova plugin add cordova-plugin-document-scanner

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

__Supported Platforms__

- Android
- iOS

**Example**  
```js
scan.scanDoc(sourceType, scanSuccess, scanError);
```

## `scan.scanDoc`

Take a photo and retrieve the image's file location:

    scan.scanDoc(0, onSuccess, onFail);

    function onSuccess(imageURI) {
        var image = document.getElementById('myImage');
        image.src = imageURI;
    }

    function onFail(message) {
        alert('Failed because: ' + message);
    }

## 'iOS Quirks'

Add permission to access photo library. Note :- Camera access permission in info.plist is already given so it is not required to add it.

<edit-config target="NSPhotoLibraryUsageDescription" file="*-Info.plist" mode="merge">
<string>need photo library access to get pictures from there</string>
</edit-config>

NOTE :- iOS has only document scan via camera for now (Any argument passed will start the camera scan). Document Scan from gallery will be available in future version
