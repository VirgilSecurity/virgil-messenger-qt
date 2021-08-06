package org.virgil.notification;

import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.RequiresApi;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;

import java.lang.ref.WeakReference;

import com.virgilsecurity.android.virgil.R;

/**
 * Asynchronous task that decrypts message and shows it as notification.
 */
public final class PushMessageHandler extends AsyncTask<Void, Void, Void> {

    private static final String TAG = "PushMessageHandler";
    private static final String MESSAGE_CHANNEL_ID = "notification_channel_message";

    private final BroadcastReceiver.PendingResult pendingResult;
    private final WeakReference<Context> contextRef;
    private final Intent intent;
    private final Bundle data;

    public static native DecryptedNotification
            decryptNotification(String recipientJid, String senderJid, String ciphertext);

    public PushMessageHandler(BroadcastReceiver.PendingResult pendingResult, Context context, Intent intent, Bundle data) {
        this.pendingResult = pendingResult;
        this.contextRef = new WeakReference<>(context);
        this.intent = intent;
        this.data = data;
    }

    @Override
    protected Void doInBackground(Void... params) {
        Log.d(TAG, "Handle message");
        String messageId = data.getString(Constants.PUSH.MESSAGE_ID);
        String senderJid = data.getString(Constants.PUSH.SENDER_JID);
        String recipientJid = data.getString(Constants.PUSH.RECIPIENT_JID);
        String ciphertext = data.getString(Constants.PUSH.CIPHERTEXT);

        //
        //  Try to decrypt encrypted message.
        //
        DecryptedNotification decryptedNotification = null;

        try {
            decryptedNotification = decryptNotification(recipientJid, senderJid, ciphertext);

        } catch (java.lang.UnsatisfiedLinkError error) {
            Log.e(TAG, "Failed to decrypt notification, due to closed application so native libraries was not loaded.");
            showNotification(messageId.hashCode(), "New Message", "Encrypted");
            return null;
        }

        //
        //  Show notification or ignore it.
        //
        switch(decryptedNotification.getResult()) {
            case DECRYPTED:
                showNotification(
                        messageId.hashCode(), decryptedNotification.getTitle(), decryptedNotification.getBody());
                break;

            case FAILED:
                Log.e(TAG, "Failed to decrypt notification.");
                showNotification(messageId.hashCode(), "New Message", "Encrypted");
                break;

            case SKIPPED:
                break;

        }

        return null;
    }

    @Override
    protected void onPostExecute(Void result) {
        super.onPostExecute(result);

        if (this.pendingResult != null) {
            this.pendingResult.finish();
        }
    }

    @RequiresApi(api = Build.VERSION_CODES.O)
    private void createNotificationChannel() {
        Log.d(TAG, "Create notification channel: " + MESSAGE_CHANNEL_ID);

        Context context = this.contextRef.get();
        CharSequence name = context.getString(R.string.message_channel_name);
        String description = context.getString(R.string.message_channel_description);
        int importance = NotificationManager.IMPORTANCE_DEFAULT;
        NotificationChannel channel = new NotificationChannel(MESSAGE_CHANNEL_ID, name, importance);
        channel.setDescription(description);

        //
        //  Register the channel with the system; you can't change the importance
        //  or other notification behaviors after this.
        //
        NotificationManager notificationManager = context.getSystemService(NotificationManager.class);
        notificationManager.createNotificationChannel(channel);
    }

    private void showNotification(int notificationId, String title, String body) {
        Log.d(TAG, "Got message: " + body);

        createNotificationChannel();

        Context context = this.contextRef.get();
        NotificationCompat.Builder builder = new NotificationCompat.Builder(context, MESSAGE_CHANNEL_ID)
                .setSmallIcon(R.drawable.icon)
                .setAutoCancel(true)
                .setContentTitle(title)
                .setContentText(body)
                .setPriority(NotificationManager.IMPORTANCE_DEFAULT);

        NotificationManagerCompat notificationManager = NotificationManagerCompat.from(context);
        notificationManager.notify(notificationId, builder.build());
    }
}
