package org.virgil.notification;

import android.app.Activity;
import android.os.AsyncTask;
import android.util.Log;

import me.pushy.sdk.Pushy;

public final class PushService {
    private static final String TAG = "Pushy";

    public static native void updatePushToken(String token);

    private static class RegisterForPushNotificationsAsync extends AsyncTask<Void, Void, Void> {
        private static final String TAG = "Pushy";

        private Activity mainActivity;

        public RegisterForPushNotificationsAsync(Activity mainActivity) {
            this.mainActivity = mainActivity;
        }

        @Override
        protected Void doInBackground(Void... params) {
            try {
                // Register the device for notifications.
                String deviceToken = Pushy.register(this.mainActivity.getApplicationContext());

                // Registration succeeded.
                Log.d(TAG, "Got the device token: " + deviceToken);

                // Pass the token to the messenger backend server.
                PushService.updatePushToken(deviceToken);

            } catch (Exception exception) {
                Log.w(TAG, "Fetching the device token from Pushy.me failed with exception", exception);
            }

            return null;
        }
    }

    public static void requestDeviceToken(Activity activity) {
        Log.d(TAG, "Requesting a device token...");

        Pushy.listen(activity);

        new RegisterForPushNotificationsAsync(activity).execute();
    }
}
