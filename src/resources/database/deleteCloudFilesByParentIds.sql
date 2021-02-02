DELETE FROM cloudFiles
WHERE cloudFiles.parentId IN (:parentIds)
