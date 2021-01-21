UPDATE cloudFiles
SET
    parentId = :parentId,
    name = :name,
    type = :type,
    size = :size,
    createdAt = :createdAt,
    updatedAt = :updatedAt,
    updatedBy = :updatedBy
WHERE id = :id
