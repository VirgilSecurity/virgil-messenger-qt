SELECT
    chats.*,
    messages.timestamp AS messageTimestamp,
    messages.authorId AS messageAuthorId,
    messages.status AS messageStatus,
    messages.body AS messageBody
FROM
    chats
LEFT JOIN messages ON chats.lastMessageId = messages.id
