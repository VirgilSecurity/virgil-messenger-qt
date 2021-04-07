PRAGMA foreign_keys=off;

CREATE TABLE _chats_new (
    id TEXT NOT NULL PRIMARY KEY,
    title TEXT NOT NULL,
    type TEXT NOT NULL,
    createdAt INT NOT NULL,
    lastMessageId TEXT,
    FOREIGN KEY(lastMessageId) REFERENCES messages(id)
);

INSERT INTO _chats_new (id, title, type, createdAt, lastMessageId)
  SELECT id, title, type, createdAt, lastMessageId
  FROM chats;

DROP TABLE chats;

ALTER TABLE _chats_new RENAME TO chats;

CREATE INDEX chatsIdxLastMessageId ON chats(lastMessageId);

PRAGMA foreign_keys=on;
