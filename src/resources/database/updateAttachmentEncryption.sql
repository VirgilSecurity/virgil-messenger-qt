UPDATE attachments
SET encryptedSize = :encryptedSize, decryptionKey = :decryptionKey, signature = :signature
WHERE id = :id
