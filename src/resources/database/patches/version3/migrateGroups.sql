PRAGMA foreign_keys=off;

CREATE TABLE _groups_new (
        id TEXT NOT NULL PRIMARY KEY,
        superOwnerId TEXT NOT NULL,
        name TEXT NOT NULL,
        encryptionMethod TEXT NOT NULL DEFAULT "epochs", -- {epochs}
        xmppRoomType TEXT NOT NULL DEFAULT "", -- JSON
        xmppRoomConfig TEXT NOT NULL DEFAULT "", -- TODO: Choose: JSON or XML (as) received from XMPP server.
        invitationStatus TEXT NOT NULL DEFAULT "none", -- {none, invited, accepted, rejected}
        cache TEXT NOT NULL DEFAULT "",
        FOREIGN KEY(id) REFERENCES chats(id) ON DELETE CASCADE
);

INSERT INTO _groups_new (id, superOwnerId, name, encryptionMethod, xmppRoomType, xmppRoomConfig, invitationStatus, cache)
    SELECT
        id,
        groupOwnerId as superOwnerId,
        chatTitle as name,
        encryptionMethod,
        xmppRoomType,
        xmppRoomConfig,
        "accepted" as invitationStatus,
        "" as cache
    FROM groups
        LEFT JOIN (
            SELECT
                groupId,
                CASE
                    WHEN memberId IS NULL THEN ''
                    ELSE memberId
                END groupOwnerId
            FROM groupMembers
            WHERE groupMembers.memberAffiliation = 'owner'
        ) ON groupId = groups.id
        LEFT JOIN (
            SELECT
                id as chatId,
                title as chatTitle
            FROM chats WHERE chatId = id
        ) ON chatId = groups.id;

DELETE FROM _groups_new WHERE superOwnerId = '';

DROP TABLE groups;

ALTER TABLE _groups_new RENAME TO groups;

PRAGMA foreign_keys=on;
