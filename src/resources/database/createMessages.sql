CREATE TABLE messages (
	id TEXT NOT NULL PRIMARY KEY,
        timestamp TEXT NOT NULL,
        chatId NOT NULL,
        authorId TEXT NOT NULL,
	status INT NOT NULL,
	body TEXT,
        FOREIGN KEY(chatId) REFERENCES chats(id)
);

CREATE INDEX messagesIdxChatId ON messages(chatId);

CREATE INDEX messagesIdxAuthorId ON messages(authorId);

CREATE INDEX messagesIdxStatus ON messages(status);
