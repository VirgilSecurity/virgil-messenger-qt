SELECT
    messages.id AS messageId,
    messages.chatId AS messageChatId,
    messages.createdAt AS messageCreatedAt,
    messages.authorId AS messageAuthorId,
    messages.isOutgoing AS messageIsOutgoing,
    messages.stage AS messageStage,
    messages.body AS messageBody,
    messages.ciphertext AS messageCiphertext,
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
    attachments.downloadStage AS attachmentDownloadStage,
    chats.type AS chatType
FROM messages
INNER JOIN chats ON chats.id = messages.chatId
LEFT JOIN attachments ON attachments.messageId = messages.id
WHERE messages.isOutgoing = 1
    AND (messages.stage = "created"
         OR messages.stage = "encrypted")
ORDER BY messages.createdAt
