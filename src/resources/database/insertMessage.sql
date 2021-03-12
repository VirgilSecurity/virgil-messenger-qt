INSERT INTO messages (id, recipientId, senderId, chatId, createdAt, isOutgoing, stage, contentType, body, ciphertext)
VALUES (:id, :recipientId, :senderId, :chatId, :createdAt, :isOutgoing, :stage, :contentType, :body, :ciphertext)
ON CONFLICT (id) DO NOTHING
