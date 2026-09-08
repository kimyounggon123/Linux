-- -u TestUser -p test

use TestDatabase;

DROP TABLE IF EXISTS TestTable;

CREATE TABLE TestTable
(
    testID INT NOT NULL PRIMARY KEY,
    testPW VARCHAR(64) NOT NULL
);


SELECT 'Complete' as '';