DELETE FROM messages
WHERE messages.chatId = :id AND contentType = :contentType
