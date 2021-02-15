UPDATE contacts
SET email = :email
WHERE userId = :userId AND email != :email
