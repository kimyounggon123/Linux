#include "MariaDBControl.hpp"


bool MariaDBControl::Connect(const char* server, const char* user, const char* password, const char* database)
{
	if (!conn)
	{
		conn = mysql_init(nullptr);
		if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) 
			return false;
	}
	return true;
}

bool MariaDBControl::ExecuteQuery(const std::string& query)
{
	FreeResult(); // 이전 결과 정리
	if (mysql_query(conn, query.c_str()) != 0) return false;
	res = mysql_store_result(conn);
	return res != nullptr;
}

MYSQL_ROW MariaDBControl::FetchRow()
{
	return res == nullptr ? nullptr : mysql_fetch_row(res);
}

MYSQL_FIELD* MariaDBControl::GetFieldName() { return res == nullptr ? nullptr : mysql_fetch_fields(res); }
unsigned int MariaDBControl::GetFieldNum() { return res == nullptr ? 0 : mysql_num_fields(res); }

void MariaDBControl::FreeResult()
{
	if (res) 
	{
		mysql_free_result(res);
		res = nullptr;
	}
	row = nullptr;
}