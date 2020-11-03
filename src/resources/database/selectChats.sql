SELECT
    chats.*,
    messages.timestamp AS messageTimestamp,
    messages.chatId AS messageChatId,
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
    attachments.extras AS attachmentExtras
FROM
    chats
LEFT JOIN messages ON chats.lastMessageId = messages.id
LEFT JOIN attachments ON chats.lastMessageId = attachments.messageId
