UPDATE messages
SET stage = 'read'
WHERE chatId = :chatId AND createdAt <= :beforeDate AND isOutgoing AND (stage = 'sent' OR stage = 'delivered');
