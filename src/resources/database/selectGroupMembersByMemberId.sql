SELECT
    groupMembers.groupId,
    groupMembers.memberId,
    groupMembers.memberNickname,
    groupMembers.memberAffiliation,
    groupMembers.invitationStatus,
    ownerMembers.memberId as groupOwnerId
FROM
    groupMembers
LEFT JOIN groupMembers AS ownerMembers ON ownerMembers.groupId = groupMembers.groupId AND ownerMembers.memberAffiliation = 'owner'
WHERE groupMembers.memberId = :memberId
