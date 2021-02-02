UPDATE cloudFiles
SET
    parentId = :parentId,
    name = :name,
    createdAt = :createdAt,
    updatedAt = :updatedAt,
    updatedBy = :updatedBy
WHERE id = :id
