UPDATE groupMembers
SET invitationStatus = :invitationStatus
WHERE groupId = :groupId AND memberId = :memberId
