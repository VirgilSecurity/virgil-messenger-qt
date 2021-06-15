package org.virgil.notification;

public class DecryptedNotification {
    private boolean isSuccess;
    private String title;
    private String body;

    DecryptedNotification() {
        this.isSuccess = false;
    }

    DecryptedNotification(String title, String body) {
        this.isSuccess = true;
        this.title = title;
        this.body = body;
    }

    public boolean isSuccess() {
        return this.isSuccess;
    }

    public String getTitle() {
        return this.title;
    }

    public String getBody() {
        return this.body;
    }
}
