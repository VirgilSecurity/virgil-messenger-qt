SELECT
    groupMembers.groupId,
    groupMembers.memberId,
    CASE groupMembers.memberNickname
        WHEN '' THEN contacts.name
        ELSE groupMembers.memberNickname
    END memberNickname,
    groupMembers.memberAffiliation,
    ownerMembers.memberId as groupOwnerId,
    contacts.username,
    contacts.email,
    contacts.phone
FROM
    groupMembers
LEFT JOIN groupMembers AS ownerMembers ON ownerMembers.groupId = groupMembers.groupId AND ownerMembers.memberAffiliation = 'owner'
LEFT JOIN contacts ON contacts.userId = groupMembers.memberId
WHERE groupMembers.groupId = :groupId
