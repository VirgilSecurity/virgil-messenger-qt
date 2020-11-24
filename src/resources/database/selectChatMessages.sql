SELECT
    messages.id AS messageId,
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
    attachments.fingerprint AS attachmentFingerprint,
    attachments.url AS attachmentUrl,
    attachments.encryptedSize AS attachmentEncryptedSize,
    attachments.extras AS attachmentExtras
FROM
    messages
LEFT JOIN attachments ON attachments.messageId = messages.id
WHERE messages.chatId = :chatId
ORDER BY messages.timestamp
