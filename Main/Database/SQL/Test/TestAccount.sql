

SELECT 'Start.' as ''; -- as 뒤에 들어가는 건 해당 log의 제목. as 구문이 없을 경우 내용이 제목이 됨.

-- accept mysql
use mysql;

-- Drop Databases
DROP DATABASE IF EXISTS TestDatabase;
DROP USER IF EXISTS 'TestUser'@'localhost';

-- Create Databases
CREATE DATABASE IF NOT EXISTS TestDatabase
    CHARACTER SET utf8mb4
    COLLATE utf8mb4_unicode_ci;

-- Create User
CREATE USER IF NOT EXISTS 'TestUser'@'localhost' IDENTIFIED BY 'test';
GRANT ALL PRIVILEGES ON TestDatabase.* TO 'TestUser'@'localhost';


-- message 출력 방식
-- 1. 일반적인 형태
SELECT 'Complete.' as ''; -- as 뒤에 들어가는 건 해당 log의 제목. as 구문이 없을 경우 내용이 제목이 됨.

-- 2. 변수 형태 
-- SET @msg = 'Database initialized'; -- 변수 msg에 메시지를 저장 후
-- SELECT CONCAT('[INFO] ', @msg) AS 'hello'; -- CONCAT으로 사용. printf와 유사.


