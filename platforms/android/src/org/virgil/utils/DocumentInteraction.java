package org.virgil.utils;

import org.qtproject.qt5.android.QtNative;

import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.util.Log;
import androidx.core.content.FileProvider;
import java.io.File;

public class DocumentInteraction
{
    private static final String TAG = "DocumentInteraction";

    public static boolean viewFile(Context context, String filePath) {
        try {
            Log.i(TAG, "File opening. filePath: " + filePath);
            Uri uri;
            if (filePath.startsWith("content://")) {
                uri = Uri.parse(filePath);
            } else {
                File file = new File(filePath);
                uri = FileProvider.getUriForFile(context, context.getPackageName() + ".provider", file);
            }
            Log.i(TAG, "uri: " + uri);

            Intent viewIntent = new Intent(Intent.ACTION_VIEW, uri);
            viewIntent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
            if (viewIntent.resolveActivity(context.getPackageManager()) != null) {
                QtNative.activity().startActivity(viewIntent);
                Log.i(TAG, "File was opened. Uri: " + uri);
                return true;
            }
            else {
                Log.w(TAG, "Failed to resolve activity. Uri: " + uri);
                return false;
            }
        } catch (Exception e) {
            Log.e(TAG, "Error: " + e.toString());
            return false;
        }
    }
}
