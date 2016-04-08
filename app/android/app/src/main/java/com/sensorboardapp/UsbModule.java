package com.sensorboardapp;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.util.Log;
import android.widget.Toast;

import com.facebook.react.bridge.LifecycleEventListener;
import com.facebook.react.bridge.ReactApplicationContext;
import com.facebook.react.bridge.ReactContext;
import com.facebook.react.bridge.ReactContextBaseJavaModule;
import com.facebook.react.bridge.ReactMethod;
import com.facebook.react.modules.toast.ToastModule;

import java.io.IOException;
import java.io.OutputStream;

public class UsbModule extends ReactContextBaseJavaModule implements LifecycleEventListener {
    private static final String TAG = "UsbModule";
    private static final String ACTION_USB_PERMISSION = "com.sensorboardapp.USB_PERMISSION";

    private UsbManager mUsbManager;

    private boolean mPermissionRequestPending;
    private Connection mConnection;
    private ReactContext mReactContext;

    private final BroadcastReceiver mPermissionReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            // TODO: Check if synchronization is needed
            // synchronized (this) {
            UsbAccessory accessory = (UsbAccessory) intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
            if (intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false)) {
                if (accessory != null) {
                    openAccessory(accessory);
                }
            } else {
                Log.d(TAG, "permission denied for accessory " + accessory);
            }
            mPermissionRequestPending = false;
            // }
        }
    };

    private final BroadcastReceiver mDetachReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "Usb detached");
            Toast toast = Toast.makeText(context, "Usb detached", Toast.LENGTH_SHORT);
            toast.show();

            UsbAccessory accessory = (UsbAccessory) intent.getParcelableExtra(UsbManager.EXTRA_ACCESSORY);
            if (accessory != null) {
                closeAccessory();
            }
        }
    };

    @Override
    public String getName() {
        return "Usb";
    }

    public UsbModule(ReactApplicationContext reactContext) {
        super(reactContext);

        Log.d(TAG, "construct UsbModule");

        mReactContext = reactContext;
        mReactContext.addLifecycleEventListener(this);

        mUsbManager = (UsbManager) mReactContext.getSystemService(Context.USB_SERVICE);
        mConnection = null;
        mPermissionRequestPending = false;

        // Register receivers
        mReactContext.registerReceiver(mDetachReceiver,
                new IntentFilter(UsbManager.ACTION_USB_ACCESSORY_DETACHED)
        );

        mReactContext.registerReceiver(mPermissionReceiver,
                new IntentFilter(ACTION_USB_PERMISSION)
        );
    }

    private void connectToAccessory() {
        if (mConnection != null) return;

        // assume only one accessory (currently safe assumption)
        UsbAccessory[] accessories = mUsbManager.getAccessoryList();
        UsbAccessory accessory = (accessories == null ? null : accessories[0]);

        if (accessory == null) return;

        if (mUsbManager.hasPermission(accessory)) {
            openAccessory(accessory);
        } else {
            requestPermission(accessory);
        }
    }

    private void requestPermission(UsbAccessory accessory) {
        // TODO: Check if synchronization is needed
        // synchronized (mUsbReceiver) {
        if (mPermissionRequestPending) return;
        mPermissionRequestPending = true;

        PendingIntent mPermissionIntent = PendingIntent.getBroadcast(
                mReactContext, 0, new Intent(ACTION_USB_PERMISSION), 0
        );
        mUsbManager.requestPermission(accessory, mPermissionIntent);
        // }
    }

    private void openAccessory(UsbAccessory accessory) {
        mConnection = new UsbConnection(mUsbManager, accessory);
    }

    private void closeAccessory() {
        if (mConnection == null)
            return;

        try {
            mConnection.close();
        } catch (IOException e) {
        } finally {
            mConnection = null;
        }
    }

    @Override
    public void onHostResume() {
        Log.i(TAG, "onHostResume");
        connectToAccessory();
    }

    @Override
    public void onHostPause() {
        Log.i(TAG, "onHostPause");
    }

    @Override
    public void onHostDestroy() {
        Log.d(TAG, "onHostDestroy");

        mReactContext.unregisterReceiver(mPermissionReceiver);
        mReactContext.unregisterReceiver(mDetachReceiver);

        if (mConnection != null) {
            try {
                mConnection.close();
            } catch (IOException e) {
            } finally {
                mConnection = null;
            }
        }
    }

    @ReactMethod
    public void send(String data) {
        if (mConnection == null) {
            Log.w(TAG, "mConnection == null");
            return;
        }

        try {
            OutputStream outputStream = mConnection.getOutputStream();
            if (outputStream == null) {
                Log.e(TAG, "outputStream == null");
                return;
            }

            outputStream.write(data.getBytes());
            outputStream.write(";".getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
