#pragma once

#include "../stdafx.h"
#include "../template/Single.h"
#include <string>
#include "../thread/Mutex.h"

static const char* s_logLevel2String[] =
{
	"TRACE",
	"DEBUG",
	"INFO",
	"WARN",
	"ERROR",
	"FATAL",
};

enum
{
	LOG_LEVEL_TRACE = 0,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARN,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_FATAL,
	
	LOG_LEVEDL_MAX,
};


class Logger : public Single<Logger>
{
public:
	static const int MAX_SIZE = 1024 * 1024 * 500;	//��־�ļ�����С(500M)

	Logger();
	virtual ~Logger();

	bool Init(const std::string& name, int logLevel);

	void WriteLog(int logLev, const char* fmt, ...);

	int GetLevel() const { return m_level; }

private:
	FILE* OpenLogFile();

	int CloseLogFile();

	void FormatTime(time_t t);

private:
	FILE* m_fp;

	int m_curSize;				//�ļ���д���С

	int m_level;				//��־�ȼ�

	std::string m_fileName;		//�ļ���

	int m_fillFileNum;			//��д���ļ���

	char m_fmtTime[64];			//�洢��ʽ����ĵ�ǰʱ��

	time_t m_lastSecond;

	Mutex m_mutex;
};


#define LOG_PREFIX(fmt)	\
	std::string msg; \
	msg.reserve(256); \
	msg.append("(").\
	append(__FILE__).\
	append(":").\
	append(std::to_string(__LINE__)).\
	append(")(").\
	append(__FUNCTION__).\
	append("):").\
	append(fmt).\
	append("\n");


#define LOG_TRACE(fmt, ...) \
if(LOG_LEVEL_TRACE >= Logger::Instance().GetLevel()) \
{ \
	LOG_PREFIX(fmt) \
	Logger::Instance().WriteLog(LOG_LEVEL_TRACE, msg.c_str(), ##__VA_ARGS__); \
}


#define LOG_DEBUG(fmt, ...) \
if(LOG_LEVEL_DEBUG >= Logger::Instance().GetLevel()) \
{ \
	LOG_PREFIX(fmt) \
	Logger::Instance().WriteLog(LOG_LEVEL_DEBUG, msg.c_str(), ##__VA_ARGS__); \
}


#define LOG_INFO(fmt, ...) \
if(LOG_LEVEL_INFO >= Logger::Instance().GetLevel()) \
{ \
	LOG_PREFIX(fmt) \
	Logger::Instance().WriteLog(LOG_LEVEL_INFO, msg.c_str(), ##__VA_ARGS__); \
}


#define LOG_WARN(fmt, ...) \
if(LOG_LEVEL_WARN >= Logger::Instance().GetLevel()) \
{ \
	LOG_PREFIX(fmt) \
	Logger::Instance().WriteLog(LOG_LEVEL_WARN, msg.c_str(), ##__VA_ARGS__); \
}


#define LOG_ERROR(fmt, ...) \
if(LOG_LEVEL_ERROR >= Logger::Instance().GetLevel()) \
{ \
	LOG_PREFIX(fmt) \
	Logger::Instance().WriteLog(LOG_LEVEL_ERROR, msg.c_str(), ##__VA_ARGS__); \
}


#define LOG_FATAL(fmt, ...) \
if(LOG_LEVEL_FATAL >= Logger::Instance().GetLevel()) \
{ \
	LOG_PREFIX(fmt) \
	Logger::Instance().WriteLog(LOG_LEVEL_FATAL, msg.c_str(), ##__VA_ARGS__); \
}