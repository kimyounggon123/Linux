#ifndef MARIADBCONTROL_H
#define MARIADBCONTROL_H

#include <mariadb/mysql.h>
#include <string>
#include <cstring>
class MariaDBControl
{
	MYSQL* conn;
	MYSQL_RES* res;
	MYSQL_ROW row;
    void FreeResult(); // 결과 free

public:
	MariaDBControl(): conn(nullptr), res(nullptr), row(nullptr)
    {}

    ~MariaDBControl() 
    {
        FreeResult();
	    if (conn) mysql_close(conn);
    }

	bool Connect(const char* server, const char* user, const char* password, const char* database);
	bool IsConnected() {return conn != nullptr;}
	
	bool ExecuteQuery(const std::string& query); // 실제 쿼리문 실행

	MYSQL_ROW FetchRow(); // 쿼리문 결과를 row 형태로 가져 옴

	// query 실행 후 호출하기
	MYSQL_FIELD* GetFieldName();
	unsigned int GetFieldNum();
};



#endif