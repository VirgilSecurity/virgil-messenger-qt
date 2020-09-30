package org.virgil.notification;

import com.virgilsecurity.android.virgil.R;

import android.app.Notification;
import android.app.NotificationManager;
import android.content.Context;
import android.content.Intent;
import android.app.PendingIntent;
import android.graphics.Color;
import android.graphics.BitmapFactory;
import android.app.NotificationChannel;

public class NotificationClient
{
    private static NotificationManager m_notificationManager;
    private static Notification.Builder m_builder;
    private static int notificationId = 0;

    public NotificationClient() {}

    public static void notify(Context context, String title, String message) {
        try {

            Intent notificationIntent = context.getPackageManager().getLaunchIntentForPackage(context.getPackageName());
            notificationIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP
                        | Intent.FLAG_ACTIVITY_SINGLE_TOP);
            PendingIntent intent = PendingIntent.getActivity(context, 0,
                        notificationIntent, 0);

            m_notificationManager = (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);

            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O) {
                int importance = NotificationManager.IMPORTANCE_DEFAULT;
                NotificationChannel notificationChannel = new NotificationChannel("Qt", "Qt Notifier", importance);
                m_notificationManager.createNotificationChannel(notificationChannel);
                m_builder = new Notification.Builder(context, notificationChannel.getId());
            } else {
                m_builder = new Notification.Builder(context);
            }

            m_builder.setSmallIcon(R.drawable.icon)
                    .setContentTitle(title)
                    .setContentText(message)
                    .setContentIntent(intent)
                    .setAutoCancel(true);

            notificationId += 2;
            m_notificationManager.notify(0, m_builder.build());
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
