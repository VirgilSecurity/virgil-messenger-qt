UPDATE chats
SET unread_message_count = 0
WHERE contact_id = :contact_id
