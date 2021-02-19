INSERT INTO contacts (userId, username)
        SELECT id, title
        FROM chats
        WHERE chats.type == "personal";
