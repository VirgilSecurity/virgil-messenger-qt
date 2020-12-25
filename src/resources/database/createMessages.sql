CREATE TABLE messages (
    id TEXT NOT NULL PRIMARY KEY,
    recipientId TEXT NOT NULL,
    senderId TEXT NOT NULL,
    senderUsername TEXT NOT NULL,
    chatId TEXT NOT NULL,
    chatType TEXT NOT NULL,
    createdAt INT NOT NULL,
    isOutgoing INT NOT NULL,
    stage TEXT NOT NULL,
    contentType TEXT NOT NULL,
    body TEXT,
    ciphertext BLOB,
    FOREIGN KEY(chatId) REFERENCES chats(id)
);

CREATE INDEX messagesIdxChatId ON messages(chatId);

CREATE INDEX messagesIdxRecipientId ON messages(recipientId);

CREATE INDEX messagesIdxSenderId ON messages(senderId);

CREATE INDEX messagesIdxStage ON messages(stage);
