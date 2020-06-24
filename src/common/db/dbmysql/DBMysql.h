#pragma once

#include "../../stdafx.h"
#include "../../common.h"
#include "mysql.h"
#include "DBResult.h"

#ifdef _WIN32
#pragma comment(lib, "libmysql.lib")
#pragma comment(lib, "mysqlclient.lib")
#endif // _WIN32


#define MAX_DBINFO_BUF	128

class DBMysql : public NonCopyable
{
public:
	DBMysql(uint32 port, const char* dbip, const char* dbuser, 
		const char* dbpwd, const char* dbname);

	~DBMysql();

	uint32 Connect();

	int Ping() { return mysql_ping(m_pMysql); }

	//ִ��query���,�����������DBResult��
	int Query(const char* query, uint32 length, DBResult* pResult);

	//��ȡ�ϴδ�����
	uint32 GetLastErrno() const { return mysql_errno(m_pMysql); }

	//��ȡ�ϴδ�����Ϣ
	const char* GetLastErrStr() const { return mysql_error(m_pMysql); }

	//��Ӱ������
	uint64 GetLastAffectRows() const { return mysql_affected_rows(m_pMysql); }

private:
	//д��ִ�н��
	int QueryResult(DBResult* pResult);

private:
	MYSQL* m_pMysql;

	uint32 m_dbport;							//���ݿ�˿�

	char m_dbip[MAX_DBINFO_BUF];				//���ݿ�ip��ַ

	char m_dbuser[MAX_DBINFO_BUF];				//���ݿ��û���

	char m_dbpwd[MAX_DBINFO_BUF];				//���ݿ�����

	char m_dbname[MAX_DBINFO_BUF];				//���ݿ���
};