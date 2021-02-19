UPDATE contacts
SET isBanned = :isBanned
WHERE userId = :userId AND isBanned != :isBanned
