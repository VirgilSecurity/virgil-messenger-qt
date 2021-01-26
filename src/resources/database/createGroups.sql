CREATE TABLE groups (
        id TEXT NOT NULL PRIMARY KEY,
        encryptionMethod TEXT NOT NULL DEFAULT 'epochs', -- {epochs}
        xmppRoomType TEXT NOT NULL, -- JSON
        xmppRoomConfig TEXT NOT NULL, -- TODO: Choose: JSON or XML (as) received from XMPP server.
        FOREIGN KEY(id) REFERENCES chats(id)
);
