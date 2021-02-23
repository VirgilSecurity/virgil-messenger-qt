SELECT
    messages.id AS messageId,
    messages.recipientId AS messageRecipientId,
    messages.senderId AS messageSenderId,
    messages.chatId AS messageChatId,
    messages.createdAt AS messageCreatedAt,
    messages.isOutgoing AS messageIsOutgoing,
    messages.stage AS messageStage,
    messages.contentType as messageContentType,
    messages.body AS messageBody,
    messages.ciphertext AS messageCiphertext,
    chats.type AS messageChatType,
    attachments.id AS attachmentId,
    attachments.type AS attachmentType,
    attachments.fingerprint AS attachmentFingerprint,
    attachments.decryptionKey AS attachmentDecryptionKey,
    attachments.signature AS attachmentSignature,
    attachments.filename AS attachmentFilename,
    attachments.localPath AS attachmentLocalPath,
    attachments.url AS attachmentUrl,
    attachments.size AS attachmentSize,
    attachments.encryptedSize AS attachmentEncryptedSize,
    attachments.extras AS attachmentExtras,
    attachments.uploadStage AS attachmentUploadStage,
    attachments.downloadStage AS attachmentDownloadStage,
    senderContacts.username as messageSenderUsername,
    recipientContacts.username as messageRecipientUsername
FROM
    messages
LEFT JOIN attachments ON attachments.messageId = messages.id
LEFT JOIN chats ON chats.id = :chatId
LEFT JOIN contacts  AS senderContacts ON senderContacts.userId = messages.senderId
LEFT JOIN contacts  AS recipientContacts ON recipientContacts.userId = messages.recipientId
WHERE messages.chatId = :chatId
ORDER BY messages.createdAt
