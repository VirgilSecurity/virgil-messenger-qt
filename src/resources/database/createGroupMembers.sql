CREATE TABLE groupMembers (
        groupId TEXT NOT NULL,
        memberId TEXT NOT NULL,
        memberAffiliation TEXT NOT NULL, -- {none, outcast, member, admin, owner}
        invitationStatus TEXT NOT NULL, -- {none, invited, accepted}
        CONSTRAINT PK_GroupMembers PRIMARY KEY (groupId, memberId),
        FOREIGN KEY(groupId) REFERENCES chats(id)
);
