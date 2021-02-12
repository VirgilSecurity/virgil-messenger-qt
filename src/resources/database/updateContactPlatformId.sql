UPDATE contacts
SET platformId = :platformId
WHERE userId = :userId AND platformId != :platformId
