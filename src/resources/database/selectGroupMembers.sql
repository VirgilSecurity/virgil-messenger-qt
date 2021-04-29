SELECT
    groupMembers.groupId,
    groupMembers.memberId,
    CASE groupMembers.memberNickname
        WHEN '' THEN contacts.name
        ELSE groupMembers.memberNickname
    END memberNickname,
    groupMembers.memberAffiliation,
    contacts.username,
    contacts.email,
    contacts.phone
FROM
    groupMembers
LEFT JOIN contacts ON contacts.userId = groupMembers.memberId
WHERE groupMembers.groupId = :groupId
