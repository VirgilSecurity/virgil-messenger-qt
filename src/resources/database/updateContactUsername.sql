UPDATE contacts
SET username = :username
WHERE userId = :userId AND username != :username
