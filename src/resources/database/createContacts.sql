CREATE TABLE contacts (
    userId TEXT NOT NULL PRIMARY KEY,
    username TEXT NOT NULL DEFAULT "",
    name TEXT NOT NULL DEFAULT "",
    email TEXT NOT NULL DEFAULT "",
    phone TEXT NOT NULL DEFAULT "",
    platformId TEXT NOT NULL DEFAULT "",
    avatarLocalPath TEXT NOT NULL DEFAULT "",
    isBanned INT NOT NULL DEFAULT 0
);

CREATE INDEX contactsIdxUserId ON contacts(userId);
CREATE INDEX contactsIdxUsername ON contacts(username);
CREATE INDEX contactsIdxEmail ON contacts(email);
CREATE INDEX contactsIdxPhone ON contacts(phone);
CREATE INDEX contactsIdxPlatformId ON contacts(platformId);
