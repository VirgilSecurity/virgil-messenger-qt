SELECT
    messages.id AS messageId,
    messages.chatId AS messageChatId,
    messages.createdAt AS messageCreatedAt,
    messages.authorId AS messageAuthorId,
    messages.authorUsername AS messageAuthorUsername,
    messages.isOutgoing AS messageIsOutgoing,
    messages.stage AS messageStage,
    messages.contentType as messageContentType,
    messages.body AS messageBody,
    messages.ciphertext AS messageCiphertext,
    chats.type AS messageChatType,
    attachments.id AS attachmentId,
    attachments.type AS attachmentType,
    attachments.fingerprint AS attachmentFingerprint,
    attachments.filename AS attachmentFilename,
    attachments.localPath AS attachmentLocalPath,
    attachments.url AS attachmentUrl,
    attachments.size AS attachmentSize,
    attachments.encryptedSize AS attachmentEncryptedSize,
    attachments.extras AS attachmentExtras,
    attachments.uploadStage AS attachmentUploadStage,
    attachments.downloadStage AS attachmentDownloadStage
FROM
    messages
LEFT JOIN attachments ON attachments.messageId = messages.id
LEFT JOIN chats ON chats.id = :chatId
WHERE messages.chatId = :chatId
ORDER BY messages.createdAt
