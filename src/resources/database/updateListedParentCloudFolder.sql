UPDATE cloudFiles
SET
    parentId = :parentId,
    name = :name,
    createdAt = :createdAt,
    updatedAt = :updatedAt,
    updatedBy = :updatedBy,
    encryptedKey = :encryptedKey,
    publicKey = :publicKey
WHERE id = :id
