package org.virgil.utils;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.provider.ContactsContract;
import android.provider.OpenableColumns;

public class Utils
{
    public static String getDisplayName(Context context, String url)
    {
        String nativeFileName = null;

        Uri uri = Uri.parse(url);
        try {
            Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                nativeFileName = cursor.getString(cursor.getColumnIndex(OpenableColumns.DISPLAY_NAME));
            }
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        return (nativeFileName == null) ? "" : nativeFileName;
    }

    public static int getFileSize(Context context, String url)
    {
        int fileSize = 0;

        Uri uri = Uri.parse(url);
        try {
            Cursor cursor = context.getContentResolver().query(uri, null, null, null, null);
            if (cursor != null && cursor.moveToFirst()) {
                fileSize = cursor.getInt(cursor.getColumnIndex(OpenableColumns.SIZE));
            }
        }
        catch (Exception e) {
            e.printStackTrace();
        }

        return fileSize;
    }

    public static String getContacts(Context context)
    {
        String fetch = "";

        Cursor phones = context.getContentResolver().query(ContactsContract.CommonDataKinds.Phone.CONTENT_URI, null, null, null, null);
        final int nameIndex = phones.getColumnIndex(ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME);
        final int phoneNumberIndex = phones.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER);
        final String sep = "\n";
        while (phones.moveToNext()) {
            final String name = phones.getString(nameIndex);
            final String phoneNumber = phones.getString(phoneNumberIndex);
            final String email = "";
            final String photo = "";
            fetch += name + sep + phoneNumber + sep + email + sep + photo + sep;
        }

        return fetch;
    }
}
