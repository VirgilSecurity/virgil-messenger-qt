INSERT OR IGNORE INTO groupMembers (groupId, memberId, memberNickname, memberAffiliation)
VALUES (:groupId, :memberId, "", :memberAffiliation)
