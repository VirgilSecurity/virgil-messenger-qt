BEGIN
   IF NOT EXISTS (SELECT userId FROM contacts WHERE userId = :userId)
   BEGIN
       INSERT INTO contacts (userId, username)
       VALUES (:userId, :username)
   END
END
