UPDATE messages
SET status = :readStatus
WHERE chatId = :chatId AND authorId = :authorId
