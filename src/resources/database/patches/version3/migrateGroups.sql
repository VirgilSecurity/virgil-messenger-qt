PRAGMA foreign_keys=off;

CREATE TABLE _groups_new (
        id TEXT NOT NULL PRIMARY KEY,
        superOwnerId TEXT NOT NULL,
        encryptionMethod TEXT NOT NULL DEFAULT "epochs", -- {epochs}
        xmppRoomType TEXT NOT NULL DEFAULT "", -- JSON
        xmppRoomConfig TEXT NOT NULL DEFAULT "", -- TODO: Choose: JSON or XML (as) received from XMPP server.
        invitationStatus TEXT NOT NULL DEFAULT "none", -- {none, invited, accepted, rejected}
        cache TEXT NOT NULL DEFAULT "",
        FOREIGN KEY(id) REFERENCES chats(id) ON DELETE CASCADE
);

INSERT INTO _groups_new (id, superOwnerId, encryptionMethod, xmppRoomType, xmppRoomConfig, invitationStatus, cache)
    SELECT id, groupMembers.ownerId, encryptionMethod, xmppRoomType, xmppRoomConfig, "accepted" as invitationStatus, "" as cache
    FROM _groups_new
        LEFT JOIN (
        SELECT
            groupId,
            CASE
                WHEN memberId IS NULL THEN ''
                ELSE memberId
            END ownerId
        FROM groupMembers
        WHERE groupMembers.memberAffiliation = 'owner'
    ) AS groupMembers ON groupMembers.groupId = _groups_new.id;

DELETE FROM _groups_new WHERE superOwnerId = '';

DROP TABLE groups;

ALTER TABLE _groups_new RENAME TO groups;

PRAGMA foreign_keys=on;
