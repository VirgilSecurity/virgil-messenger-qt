package org.virgil.main;

import android.app.Activity;
import android.os.Bundle;
import android.util.Log;

import org.qtproject.qt5.android.bindings.QtActivity;

import org.virgil.notification.PushService;

public class MainActivity extends QtActivity {
    private static final String TAG = "MainActivity";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.d(TAG, "Application was started.");

        PushService.requestDeviceToken(this);
    }
}
