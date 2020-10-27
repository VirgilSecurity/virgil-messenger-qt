UPDATE chats
SET unreadMessageCount = 0
WHERE contactId = :contactId
