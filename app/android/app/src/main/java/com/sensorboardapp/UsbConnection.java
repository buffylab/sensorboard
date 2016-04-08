package com.sensorboardapp;

import java.io.FileDescriptor;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.hardware.usb.UsbAccessory;
import android.hardware.usb.UsbManager;
import android.os.ParcelFileDescriptor;
import android.util.Log;

public class UsbConnection extends Connection {
    private static final String TAG = "UsbConnection";

    FileInputStream mInputStream;
    FileOutputStream mOutputStream;
    ParcelFileDescriptor mFileDescriptor;

    public UsbConnection(UsbManager usbManager, UsbAccessory accessory) {
        mFileDescriptor = usbManager.openAccessory(accessory);
        if (mFileDescriptor == null) {
            Log.e(TAG, "Cannot find file descriptor");
            return;
        }

        FileDescriptor fd = mFileDescriptor.getFileDescriptor();
        mInputStream = new FileInputStream(fd);
        mOutputStream = new FileOutputStream(fd);
    }

    @Override
    public InputStream getInputStream() {
        return mInputStream;
    }

    @Override
    public OutputStream getOutputStream() {
        return mOutputStream;
    }

    public void close() throws IOException {
        if (mFileDescriptor != null) {
            mFileDescriptor.close();
        }
    }
}

