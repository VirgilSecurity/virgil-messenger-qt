CREATE TABLE cloudFiles (
    id TEXT NOT NULL PRIMARY KEY,
    parentId TEXT NOT NULL,
    name TEXT NOT NULL,
    isFolder INT NOT NULL,
    type TEXT NOT NULL,
    size INT NOT NULL,
    createdAt INT NOT NULL,
    updatedAt INT NOT NULL,
    updatedBy TEXT NOT NULL,
    encryptedKey BLOB NOT NULL,
    publicKey BLOB,
    localPath TEXT,
    fingerprint BLOB
);

CREATE INDEX cloudFilesParentId ON cloudFiles(parentId);
