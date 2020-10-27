CREATE TABLE chats (
        id TEXT NOT NULL PRIMARY KEY,
	timestamp TEXT NOT NULL,
	contactId TEXT NOT NULL,
	lastMessageId TEXT,
	unreadMessageCount INT NOT NULL,
	FOREIGN KEY(contactId) REFERENCES contacts(id),
	FOREIGN KEY(lastMessageId) REFERENCES messages(id)
);

CREATE INDEX chatsIdxContactId ON chats(contactId);

CREATE INDEX chatsIdxLastMessageId ON chats(lastMessageId);

CREATE INDEX chatsIdxUnreadMessageCount ON chats(unreadMessageCount);
