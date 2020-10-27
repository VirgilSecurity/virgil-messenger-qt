CREATE TABLE messages (
	id TEXT NOT NULL PRIMARY KEY,
	timestamp TEXT NOT NULL,
        authorId TEXT NOT NULL,
	status INT NOT NULL,
	body TEXT,
        FOREIGN KEY(authorId) REFERENCES contacts(id)
);

CREATE INDEX messagesIdxAuthorId ON messages(authorId);

CREATE INDEX messagesIdxStatus ON messages(status);
