CREATE TABLE groups (
        id TEXT NOT NULL PRIMARY KEY,
        superOwnerId TEXT NOT NULL,
        encryptionMethod TEXT NOT NULL DEFAULT "epochs", -- {epochs}
        xmppRoomType TEXT NOT NULL DEFAULT "", -- JSON
        xmppRoomConfig TEXT NOT NULL DEFAULT "", -- TODO: Choose: JSON or XML (as) received from XMPP server.
        invitationStatus TEXT NOT NULL DEFAULT "none", -- {none, invited, accepted, rejected}
        cache TEXT NOT NULL DEFAULT "",
        FOREIGN KEY(id) REFERENCES chats(id) ON DELETE CASCADE
);
