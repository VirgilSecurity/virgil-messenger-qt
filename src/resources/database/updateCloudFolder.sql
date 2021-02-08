UPDATE cloudFiles
SET
    parentId = :parentId,
    name = :name,
    isFolder = :isFolder,
--  no type and size
    createdAt = :createdAt,
    updatedAt = :updatedAt,
    updatedBy = :updatedBy,
    encryptedKey = :encryptedKey,
    publicKey = :publicKey,
    localPath = :localPath
--  no fingerprint
WHERE id = :id
