#pragma once

#include "stdafx.h"
#include "DBBase.h"
#include "thread/ThreadPool.h"

namespace chaos
{
	namespace db
	{
		//���ݿ�����
		enum DBType
		{
			DBT_MYSQL = 1,
			DBT_REDIS,
		};	

		static const char* DBName[] = { "mysql", "redis" };


		//���ݿ�����
		struct MysqlConfig														
		{
			char dbip[MAX_DB_IP];
			uint32 dbport;				
			char dbuser[MAX_DB_USER];				//�û���
			char dbpwd[MAX_DB_PWD];					//����
			char dbname[MAX_DB_NAME];				//���ݿ���
			char dbcharacter[MAX_DB_CHARACTER];		//�ַ���
		};

		struct RedisConfig
		{
			char dbip[MAX_DB_IP];
			uint32 dbport;
		};

		union DBConfig
		{
			MysqlConfig mysqlConfig;
			RedisConfig redisConfig;
		};


		typedef std::function<void(const std::string & cmd, DBResultBase& result, int errorno)> DBQueryCb;

		struct DBRequest
		{
			std::string cmd;
			DBQueryCb requestCb;
		};


		class DBPool : public NonCopyable
		{
		public:
			const int DB_PING_TIMEMS = 10000;		//ÿ10s PINGһ�����ݿ�
			const int DB_PING_MAX = 10;				//PING������,�����ô����ͷ���������

			DBPool(const DBConfig& dbConfig, short dbType, int maxConn = 4);
			~DBPool();

			void Start();

			void Stop();

			void Query(const std::string& cmd, const DBQueryCb& cb);

			int GetLiveConnNum() const { return m_LiveConnNum; }

			//�����е��������ָ���maxConn
			void Recover();

		private:
			void Work();

			DBBase* CreateDBBase();

			DBResultBase* CreateDBResultBase();

		private:
			//std::vector<DBBase*> m_pool;								//�������ݿ�����

			const int m_maxConn;										//������

			const short m_dbType;										//���е����ݿ�����

			DBConfig m_dbConfig;										//���ݿ�����

			std::atomic<int> m_LiveConnNum;								//������������

			std::atomic<bool> m_started;

			std::unique_ptr<ThreadPool> m_threadPool;					//�����߳�(ÿһ���߳�ά��һ������)

			std::queue<DBRequest> m_rq;									//�������

			Mutex m_mutex;

			Condition m_cond;
		};
	}
}