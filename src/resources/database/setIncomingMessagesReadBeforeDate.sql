UPDATE messages
SET stage = 'read'
WHERE chatId = :chatId AND createdAt <= :beforeDate AND NOT isOutgoing AND stage = 'decrypted';
