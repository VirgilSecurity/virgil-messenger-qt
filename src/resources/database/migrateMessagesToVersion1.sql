PRAGMA foreign_keys=off;

ALTER TABLE messages RENAME TO _messages_old;

CREATE TABLE messages (
    id TEXT NOT NULL PRIMARY KEY,
    recipientId TEXT NOT NULL,
    senderId TEXT NOT NULL,
    chatId TEXT NOT NULL,
    createdAt INT NOT NULL,
    isOutgoing INT NOT NULL,
    stage TEXT NOT NULL,
    contentType TEXT NOT NULL,
    body TEXT,
    ciphertext BLOB,
    FOREIGN KEY(chatId) REFERENCES chats(id)
);

DROP INDEX messagesIdxChatId;
DROP INDEX messagesIdxRecipientId;
DROP INDEX messagesIdxSenderId;
DROP INDEX messagesIdxStage;

CREATE INDEX messagesIdxChatId ON messages(chatId);
CREATE INDEX messagesIdxRecipientId ON messages(recipientId);
CREATE INDEX messagesIdxSenderId ON messages(senderId);
CREATE INDEX messagesIdxStage ON messages(stage);

INSERT INTO messages (id, recipientId, senderId, chatId, createdAt, isOutgoing, stage, contentType, body, ciphertext)
  SELECT id, recipientId, senderId, chatId, createdAt, isOutgoing, stage, contentType, body, ciphertext
  FROM _messages_old;

DROP TABLE _messages_old;

PRAGMA foreign_keys=on;
