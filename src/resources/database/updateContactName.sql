UPDATE contacts
SET name = :name
WHERE userId = :userId AND name != :name
