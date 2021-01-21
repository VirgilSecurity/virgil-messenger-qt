UPDATE cloudFiles
SET
    parentId = :parentId,
    name = :name,
    isFolder = :isFolder,
    type = :type,
    size = :size,
    createdAt = :createdAt,
    updatedAt = :updatedAt,
    updatedBy = :updatedBy,
    encryptedKey = :encryptedKey,
    publicKey = :publicKey,
    localPath = :localPath,
    fingerprint = :fingerprint
WHERE id = :id
