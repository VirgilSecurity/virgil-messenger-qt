CREATE TABLE contacts (
	id TEXT NOT NULL PRIMARY KEY,
	type INT NOT NULL,
	name TEXT NOT NULL,
	avatarUrl TEXT
);

CREATE INDEX contactsIdxType ON contacts(type);

CREATE INDEX contactsIdxName ON contacts(name);
