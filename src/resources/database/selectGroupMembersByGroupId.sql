SELECT
    *,
    groups.onwerId as groupOwnerId
FROM
    groupMembers
LEFT JOIN groups ON groups.id = :groupId
WHERE groupMembers.groupId = :groupId
