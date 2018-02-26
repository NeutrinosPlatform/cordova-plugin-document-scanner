package com.neutrinos.plugin;

import org.apache.cordova.CallbackContext;
import org.apache.cordova.CordovaPlugin;
import org.apache.cordova.PluginResult;
import org.json.JSONArray;
import org.json.JSONException;

import android.Manifest;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;

import com.scanlibrary.ScanActivity;
import com.scanlibrary.ScanConstants;

public class Scan extends CordovaPlugin {

    private static final int REQUEST_CODE = 99;
    private static final int PHOTOLIBRARY = 0; // Choose image from picture library (same as SAVEDPHOTOALBUM for Android)
    private static final int CAMERA = 1; // Take picture from camera

    private int srcType;

    public CallbackContext callbackContext;

    public static final int PERMISSION_DENIED_ERROR = 20;
    public static final int TAKE_PIC_SEC = 0;
    public static final int SAVE_TO_ALBUM_SEC = 1;

    protected final static String[] permissions = { Manifest.permission.CAMERA,
            Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE };

    @Override
    public boolean execute(String action, JSONArray args, CallbackContext callbackContext) throws JSONException {

        if (action.equals("scanDoc")) {

            this.srcType = CAMERA;

            //Take the values from the arguments if they're not already defined (this is tricky)
            this.srcType = args.getInt(0);

            this.callbackContext = callbackContext;

            cordova.setActivityResultCallback(this);

            int preference = 0;
            try {
                if (this.srcType == CAMERA) {
                    preference = ScanConstants.OPEN_CAMERA;
                } else if (this.srcType == PHOTOLIBRARY) {
                    preference = ScanConstants.OPEN_MEDIA;
                }
                Intent intent = new Intent(cordova.getActivity().getApplicationContext(), ScanActivity.class);
                intent.putExtra(ScanConstants.OPEN_INTENT_PREFERENCE, preference);
                cordova.getActivity().startActivityForResult(intent, REQUEST_CODE);
            } catch (IllegalArgumentException e) {
                this.callbackContext.error("Illegal Argument Exception");
                PluginResult r = new PluginResult(PluginResult.Status.ERROR);
                this.callbackContext.sendPluginResult(r);
            }

            return true;

        } else {

            return false;

        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_CODE && resultCode == cordova.getActivity().RESULT_OK) {
            Uri uri = data.getExtras().getParcelable(ScanConstants.SCANNED_RESULT);
            if (uri != null) {
                String fileLocation = FileHelper.getRealPath(uri, this.cordova);
                this.callbackContext.success("file://" + fileLocation);
            } else {
                this.callbackContext.error("null data from scan libary");
            }
        }
    }
}
