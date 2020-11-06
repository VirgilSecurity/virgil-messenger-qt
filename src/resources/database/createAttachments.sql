CREATE TABLE attachments (
        id TEXT NOT NULL,
        messageId TEXT NOT NULL,
        type INT NOT NULL,
        status INT NOT NULL,
        filename TEXT NOT NULL,
        size INT NOT NULL,
        localPath TEXT,
        url TEXT,
        encryptedSize INT NOT NULL,
        extras TEXT,
        FOREIGN KEY(messageId) REFERENCES messages(id)
);

CREATE INDEX attachmentsIdxMessageId ON attachments(messageId);

CREATE INDEX attachmentsIdxType ON attachments(type);

CREATE INDEX attachmentsIdxStatus ON attachments(status);
