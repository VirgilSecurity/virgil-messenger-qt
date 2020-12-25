CREATE TABLE messages (
    id TEXT NOT NULL PRIMARY KEY,
    chatId TEXT NOT NULL,
    chatType TEXT NOT NULL,
    createdAt INT NOT NULL,
    authorId TEXT NOT NULL,
    authorUsername TEXT NOT NULL,
    isOutgoing INT NOT NULL,
    stage TEXT NOT NULL,
    contentType TEXT NOT NULL,
    body TEXT,
    ciphertext BLOB,
    FOREIGN KEY(chatId) REFERENCES chats(id)
);

CREATE INDEX messagesIdxChatId ON messages(chatId);

CREATE INDEX messagesIdxAuthorId ON messages(authorId);

CREATE INDEX messagesIdxStage ON messages(stage);
