SELECT
    messages.id AS messageId,
    messages.timestamp AS messageTimestamp,
    messages.authorId AS messageAuthorId,
    messages.status AS messageStatus,
    messages.body AS messageBody,
    attachments.id AS attachmentId,
    attachments.type AS attachmentType,
    attachments.status AS attachmentStatus,
    attachments.filename AS attachmentFilename,
    attachments.size AS attachmentSize,
    attachments.localPath AS attachmentLocalPath,
    attachments.url AS attachmentUrl,
    attachments.extras AS attachmentExtras,
    chats.contactId AS contactId,
    :userId AS senderId,
    chats.contactId AS recipientId
FROM
    messages
INNER JOIN chats ON chats.id = messages.chatId
LEFT JOIN attachments ON attachments.messageId = messages.id
WHERE (messages.status = :failedStatus OR messages.status = :createdStatus) AND messages.authorId == :userId
ORDER BY messages.timestamp
