SELECT
    messages.id AS messageId,
    messages.recipientId AS messageRecipientId,
    messages.senderId AS messageSenderId,
    messages.senderUsername AS messageSenderUsername,
    messages.chatId AS messageChatId,
    messages.createdAt AS messageCreatedAt,
    messages.isOutgoing AS messageIsOutgoing,
    messages.stage AS messageStage,
    messages.contentType as messageContentType,
    messages.body AS messageBody,
    messages.ciphertext AS messageCiphertext,
    chats.type AS messageChatType,
    chats.title AS messageRecipientUsername,
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
FROM messages
INNER JOIN chats ON chats.id = messages.chatId
LEFT JOIN attachments ON attachments.messageId = messages.id
WHERE messages.isOutgoing = 1
    AND (messages.stage = "created"
         OR messages.stage = "encrypted")
ORDER BY messages.createdAt
