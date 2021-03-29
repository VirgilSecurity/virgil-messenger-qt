SELECT
    chats.id,
    chats.type,
    chats.createdAt,
    chats.lastMessageId,
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
    recipientContacts.username as messageRecipientUsername,
    coalesce(notReadMessages.unreadMessageCount, 0) as unreadMessageCount,
    CASE chats.type
        WHEN 'personal' THEN chats.title
        WHEN 'group' THEN groups.name
    END title
FROM
    chats
LEFT JOIN messages ON chats.lastMessageId = messages.id
LEFT JOIN attachments ON chats.lastMessageId = attachments.messageId
LEFT JOIN contacts  AS senderContacts ON senderContacts.userId = messages.recipientId
LEFT JOIN contacts  AS recipientContacts ON recipientContacts.userId = messages.senderId
LEFT JOIN groups  ON groups.id = chats.id
LEFT JOIN
    (
    SELECT
        chatId, count(chatId) as unreadMessageCount
    FROM
        messages
    WHERE messages.stage = 'decrypted'
    ) AS notReadMessages ON chats.id = notReadMessages.chatId
