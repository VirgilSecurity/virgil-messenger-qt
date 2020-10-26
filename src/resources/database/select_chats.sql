SELECT
    chats.*,
    contacts.type AS contact_type,
    IFNULL(contacts.name, chats.contact_id) AS contact_name,
    contacts.avatar_url AS contact_avatar_url,
    messages.timestamp AS message_timestamp,
    messages.author_id AS message_author_id,
    messages.status AS message_status,
    messages.body AS message_body
FROM
    chats
LEFT JOIN contacts oN chats.contact_id = contacts.id
LEFT JOIN messages ON chats.last_message_id = messages.id
