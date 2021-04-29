INSERT INTO contacts (userId, username)
VALUES (:userId, :username)
ON CONFLICT (userId) DO UPDATE SET
    username = :username;

