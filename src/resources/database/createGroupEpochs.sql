-- Cache table that handles group epoch (a serial number plus a key)
CREATE TABLE groupEpochs (
        groupId TEXT NOT NULL PRIMARY KEY,
        epochNum INT NOT NULL,
        epoch BLOB NOT NULL,
        FOREIGN KEY(groupId) REFERENCES chats(id)
);
