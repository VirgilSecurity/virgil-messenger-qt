CREATE TABLE IF NOT EXISTS attachments (
	message_id TEXT NOT NULL,
	type INT NOT NULL,
	status INT NOT NULL,
	filename TEXT NOT NULL,
	size INT NOT NULL,
	local_path TEXT,
	url TEXT,
	extras TEXT,
	FOREIGN KEY(message_id) REFERENCES messages(id)
);

CREATE INDEX IF NOT EXISTS attachments_idx_message_id ON attachments(message_id);

CREATE INDEX IF NOT EXISTS attachments_idx_type ON attachments(type);

CREATE INDEX IF NOT EXISTS attachments_idx_status ON attachments(status);
