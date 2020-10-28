SELECT
    messages.*,
    attachments.id AS attachmentId,
    attachments.type AS attachmentType,
    attachments.status AS attachmentStatus,
    attachments.filename AS attachmentFilename,
    attachments.size AS attachmentSize,
    attachments.localPath AS attachmentLocalPath,
    attachments.url AS attachmentUrl,
    attachments.extras AS attachmentExtras
FROM
    messages
LEFT JOIN attachments ON attachments.messageId = messages.id
WHERE messages.chatId = :chatId
