package org.virgil.notification;

import android.app.Activity;
import android.util.Log;

import com.google.android.gms.tasks.Task;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.firebase.messaging.FirebaseMessaging;



public final class PushService {
    private static final String TAG = "Firebase";

    public static native void updatePushToken(String token);

    public static void requestDeviceToken(Activity activity) {
        FirebaseMessaging.getInstance().getToken()
            .addOnCompleteListener(new OnCompleteListener < String > () {
                @Override
                public void onComplete(Task < String > task) {
                    if (!task.isSuccessful()) {
                        Log.w(TAG, "Fetching FCM registration token failed", task.getException());
                        return;
                    }

                    //
                    // Get new FCM registration token.
                    //
                    String token = task.getResult();

                    Log.d(TAG, "Got token: " + token);

                    updatePushToken(token);
                }
            });
    }
}
