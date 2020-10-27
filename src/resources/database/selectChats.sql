SELECT
    chats.*,
    contacts.type AS contactType,
    IFNULL(contacts.name, chats.contactId) AS contactName,
    contacts.avatarUrl AS contactAvatarUrl,
    messages.timestamp AS messageTimestamp,
    messages.authorId AS messageAuthorId,
    messages.status AS messageStatus,
    messages.body AS messageBody
FROM
    chats
LEFT JOIN contacts ON chats.contactId = contacts.id
LEFT JOIN messages ON chats.lastMessageId = messages.id
