UPDATE contacts
SET avatarLocalPath = :avatarLocalPath
WHERE userId = :userId AND avatarLocalPath != :avatarLocalPath
