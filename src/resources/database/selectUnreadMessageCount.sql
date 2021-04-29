SELECT COUNT(chatId) as unreadMessageCount
FROM messages
WHERE messages.chatId = :id  AND messages.stage = 'decrypted'
