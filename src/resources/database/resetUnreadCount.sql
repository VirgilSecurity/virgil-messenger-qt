UPDATE messages
SET stage = 'read'
WHERE chatId = :id AND NOT isOutgoing AND stage = 'decrypted';
