CREATE TABLE groups (
        id TEXT NOT NULL PRIMARY KEY,
        encryptionMethod TEXT NOT NULL DEFAULT "epochs", -- {epochs}
        xmppRoomType TEXT NOT NULL DEFAULT "", -- JSON
        xmppRoomConfig TEXT NOT NULL DEFAULT "", -- TODO: Choose: JSON or XML (as) received from XMPP server.
        FOREIGN KEY(id) REFERENCES chats(id) ON DELETE CASCADE
);
