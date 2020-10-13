CREATE TABLE contacts (
	id TEXT NOT NULL PRIMARY KEY,
	type INT NOT NULL,
	name TEXT NOT NULL,
	avatar_url TEXT
);

CREATE INDEX contacts_idx_type ON contacts(type);

CREATE INDEX contacts_idx_name ON contacts(name);
