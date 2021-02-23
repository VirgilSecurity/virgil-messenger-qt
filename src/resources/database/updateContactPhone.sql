UPDATE contacts
SET phone = :phone
WHERE userId = :userId AND phone != :phone
