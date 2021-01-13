UPDATE chats
SET lastMessageId = :lastMessageId, unreadMessageCount = :unreadMessageCount
WHERE id = :id
