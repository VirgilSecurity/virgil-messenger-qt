package org.virgil.notification;

import org.virgil.notification.DecryptedNotification;
import com.virgilsecurity.android.virgil.R;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;

import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import com.google.android.gms.tasks.Task;
import com.google.android.gms.tasks.OnCompleteListener;
import com.google.firebase.messaging.FirebaseMessagingService;
import com.google.firebase.messaging.RemoteMessage;
import com.google.firebase.messaging.FirebaseMessaging;

public class FirebaseMessageReceiver extends FirebaseMessagingService {

    private static final String TAG = "FirebaseMessageReceiver";

    private static final String MESSAGE_CHANNEL_ID = "notification_channel_message";

    private static native void updatePushToken(String token);

    private static native DecryptedNotification decryptNotification(String recipientJid, String senderJid, String ciphertext);

    /**
     * Request push token.
     */
    public static void init() {
        FirebaseMessaging.getInstance().getToken()
            .addOnCompleteListener(new OnCompleteListener < String > () {
                @Override
                public void onComplete(Task < String > task) {
                    if (!task.isSuccessful()) {
                        Log.w(TAG, "Fetching FCM registration token failed", task.getException());
                        return;
                    }

                    // Get new FCM registration token
                    String token = task.getResult();

                    Log.d(TAG, "Got token: " + token);

                    updatePushToken(token);
                }
            });
    }

    @Override
    public void onCreate() {
        super.onCreate();
        createNotificationChannel();
    }

    private void createNotificationChannel() {
        // Create the NotificationChannel, but only on API 26+ because
        // the NotificationChannel class is new and not in the support library
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            CharSequence name = getString(R.string.message_channel_name);
            String description = getString(R.string.message_channel_description);
            int importance = NotificationManager.IMPORTANCE_DEFAULT;
            NotificationChannel channel = new NotificationChannel(MESSAGE_CHANNEL_ID, name, importance);
            channel.setDescription(description);
            // Register the channel with the system; you can't change the importance
            // or other notification behaviors after this
            NotificationManager notificationManager = getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }
    }

    /**
     * There are two scenarios when onNewToken is called:
     * 1) When a new token is generated on initial app startup
     * 2) Whenever an existing token is changed
     * Under #2, there are three scenarios when the existing token is changed:
     * A) App is restored to a new device
     * B) User uninstalls/reinstalls the app
     * C) User clears app data
     */
    @Override
    public void onNewToken(String token) {
        Log.d(TAG, "Refreshed token: " + token);

        // If you want to send messages to this application instance or
        // manage this apps subscriptions on the server side, send the
        // FCM registration token to your app server.
        updatePushToken(token);
    }

    // Override onMessageReceived() method to extract the
    // title and
    // body from the message passed in FCM
    @Override
    public void
    onMessageReceived(RemoteMessage remoteMessage) {
        Log.d(TAG, "Try to decrypt Notification.");

        //
        //  Expect a data notification with a structure:
        //      {
        //          "recipient" : "Recipient XMPP JID",
        //          "sender" : "Sender XMPP JID",
        //          "ciphertext" : "Base64 encoded cipher text",
        //      }
        //
        if (remoteMessage.getData().isEmpty()) {
            Log.w(TAG, "Data is empty.");
            return;
        }

        String senderJid = remoteMessage.getData().get("sender");
        String recipientJid = remoteMessage.getData().get("recipient");
        String ciphertext = remoteMessage.getData().get("ciphertext");

        if (senderJid.isEmpty()) {
            Log.w(TAG, "Required field 'from' is missing.");
            return;
        } else {
            Log.d(TAG, "Sender JID: " + senderJid);
        }

        if (recipientJid.isEmpty()) {
            Log.w(TAG, "Required field 'to' is missing.");
            return;
        } else {
            Log.d(TAG, "Recipient JID: " + recipientJid);
        }

        if (ciphertext.isEmpty()) {
            Log.w(TAG, "Required field 'ciphertext' is missing.");
            return;
        } else {
            Log.d(TAG, "Ciphertext: " + ciphertext);
        }

        //
        //  Try to decrypt encrypted message.
        //
        DecryptedNotification decryptedNotification = decryptNotification(recipientJid, senderJid, ciphertext);

        if (!decryptedNotification.isSuccess()) {
            Log.w(TAG, "Failed to decrypt notification.");
            return;
        }

        Log.d(TAG, "Notification was decrypted.");
        showNotification(remoteMessage.getSentTime(), decryptedNotification.getTitle(), decryptedNotification.getBody());
    }

    // Method to display the notifications
    public void showNotification(Long notificationId, String title, String body) {
        Log.d(TAG, "Got message: " + body);

        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, MESSAGE_CHANNEL_ID)
                .setSmallIcon(R.drawable.icon)
                .setAutoCancel(true);
        builder.setContentTitle(title);
        builder.setContentText(body);
        builder.setPriority(NotificationCompat.PRIORITY_MAX);

        NotificationManagerCompat notificationManager = NotificationManagerCompat.from(this);
        notificationManager.notify(notificationId.intValue(), builder.build());
    }
}
