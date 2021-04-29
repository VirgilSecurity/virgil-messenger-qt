UPDATE messages
SET stage = :stage
WHERE id = :id AND isOutgoing = 0;
