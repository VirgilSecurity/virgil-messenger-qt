UPDATE messages
SET status = 3 -- read
WHERE chatId = :chatId AND authorId = :authorId
