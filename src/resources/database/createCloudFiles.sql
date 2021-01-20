CREATE TABLE cloudFiles (
    id TEXT NOT NULL PRIMARY KEY,
    parentId TEXT NOT NULL,
    name TEXT NOT NULL,
    isFolder INT NOT NULL,
    type TEXT,
    size INT,
    createdAt INT NOT NULL,
    updatedAt INT NOT NULL,
    updatedBy TEXT NOT NULL,
    encryptedKey BLOB,
    publicKey BLOB,
    localPath TEXT,
    fingerprint BLOB
);

CREATE INDEX cloudFilesParentId ON cloudFiles(parentId);
