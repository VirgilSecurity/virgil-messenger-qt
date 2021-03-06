SELECT
    id AS cloudFileId,
    parentId AS cloudFileParentId,
    name AS cloudFileName,
    isFolder AS cloudFileIsFolder,
    type AS cloudFileType,
    size AS cloudFileSize,
    createdAt AS cloudFileCreatedAt,
    updatedAt AS cloudFileUpdatedAt,
    updatedBy AS cloudFileUpdatedBy,
    encryptedKey AS cloudFileEncryptedKey,
    publicKey AS cloudFilePublicKey,
    localPath AS cloudFileLocalPath,
    fingerprint AS cloudFileFingerprint,
    sharedGroupId AS cloudFileSharedGroupId
FROM
    cloudFiles
WHERE
    cloudFiles.parentId = :folderId
