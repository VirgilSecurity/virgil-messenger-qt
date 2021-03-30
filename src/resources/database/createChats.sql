CREATE TABLE chats (
    id TEXT NOT NULL PRIMARY KEY,
    title TEXT NOT NULL,
    type TEXT NOT NULL,
    createdAt INT NOT NULL,
    lastMessageId TEXT,
    FOREIGN KEY(lastMessageId) REFERENCES messages(id)
);

CREATE INDEX chatsIdxLastMessageId ON chats(lastMessageId);
