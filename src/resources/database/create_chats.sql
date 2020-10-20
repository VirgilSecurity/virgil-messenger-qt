CREATE TABLE chats (
        id TEXT NOT NULL PRIMARY KEY,
	timestamp TEXT NOT NULL,
	contact_id TEXT NOT NULL,
	last_message_id TEXT,
	unread_message_count INT NOT NULL,
	FOREIGN KEY(contact_id) REFERENCES contacts(id),
	FOREIGN KEY(last_message_id) REFERENCES messages(id)
);

CREATE INDEX chats_idx_contact_id ON chats(contact_id);

CREATE INDEX chats_idx_last_message_id ON chats(last_message_id);
