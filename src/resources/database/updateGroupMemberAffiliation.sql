INSERT INTO groupMembers (groupId, memberId, memberNickname, memberAffiliation)
VALUES (:groupId, :memberId, "", :memberAffiliation)
ON CONFLICT (groupId, memberId) DO UPDATE SET
    memberAffiliation = :memberAffiliation;
