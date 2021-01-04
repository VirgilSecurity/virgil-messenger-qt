CREATE TABLE attachments (
        id TEXT NOT NULL PRIMARY KEY,
        messageId TEXT NOT NULL,
        type TEXT NOT NULL,
        fingerprint TEXT,
        decryptionKey BLOB,
        filename TEXT NOT NULL,
        localPath TEXT,
        url TEXT,
        size INT NOT NULL,
        encryptedSize INT NOT NULL,
        extras TEXT,
        uploadStage TEXT NOT NULL,
        downloadStage TEXT NOT NULL,
        FOREIGN KEY(messageId) REFERENCES messages(id)
);

CREATE INDEX attachmentsIdxMessageId ON attachments(messageId);

CREATE INDEX attachmentsIdxType ON attachments(type);

CREATE INDEX attachmentsIdxUploadStage ON attachments(uploadStage);

CREATE INDEX attachmentsIdxDownloadStage ON attachments(downloadStage);
