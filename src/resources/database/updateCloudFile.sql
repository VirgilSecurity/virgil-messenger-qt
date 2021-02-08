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
--  no encrypted and public keys
    localPath = :localPath,
    fingerprint = :fingerprint
WHERE id = :id
