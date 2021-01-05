UPDATE attachments
SET encryptedSize = :encryptedSize, decryptionKey = :decryptionKey
WHERE id = :id
