#pragma once


#include "stdafx.h"
#include "Mutex.h"

class Condition :public NonCopyable
{
public:
	enum CondState
	{
		CS_COND_NORMAL = 0,		
		CS_COND_BROADCAST,		//�㲥,���ѵȴ������������������߳�
		CS_COND_SINGLE,			//���ѵ����߳�
	};

	Condition(Mutex& mutex):
		m_mutex(mutex)
#ifdef _WIN32
		,m_waitThreadNum(0)
		,m_state(CS_COND_NORMAL)
#endif // _WIN32

	{
#ifdef _WIN32
		m_cond = CreateEvent(NULL, TRUE, FALSE, NULL);
#else
		pthread_cond_init(&m_cond, NULL);
#endif // _WIN32
		
	}


	~Condition()
	{
#ifdef _WIN32
		CloseHandle(m_cond);
#else
		pthread_cond_destroy(&m_cond);
#endif // _WIN32

	}


	int CondWait(int timeoutMs = -1)
	{
#ifdef _WIN32
		bool forever = false;
		if (0 > timeoutMs)
			timeoutMs = INFINITE;

		int ret = 0;

		//����,��UnLock,��Lock,����ģ��pthread_cond_wait
		m_mutex.UnLock();

		do
		{
			++m_waitThreadNum;

			ret = WaitForSingleObject(m_cond, timeoutMs);

			--m_waitThreadNum;

			if (CS_COND_SINGLE == m_state ||
				(CS_COND_BROADCAST == m_state && 0 == m_waitThreadNum)
				)
			{
				ResetEvent(m_cond);
				m_state = CS_COND_NORMAL;
			}

			//�������� ������linux pthread_cond_wait
			if (WAIT_TIMEOUT == ret && INFINITE == timeoutMs)
				forever = true;

		} while (forever);

		m_mutex.Lock();

		return ret;
#else
		if (timeoutMs >= 0)
		{
			struct timespec timeSpec;
			timeSpec.tv_sec = timeoutMs / SEC2MSEC;
			timeSpec.tv_nsec = (timeoutMs % SEC2MSEC) * SEC2MSEC * SEC2MSEC;
			
			return pthread_cond_timedwait(&m_cond, m_mutex.GetMutex(), &timeSpec);
		}
		return pthread_cond_wait(&m_cond, m_mutex.GetMutex());
#endif // _WIN32

	}


	int CondSignal()
	{
#ifdef _WIN32
		//��ǰ���ڹ㲥״̬,���еȴ��е��̶߳��ᱻ����
		//û�б�Ҫ�ٷ�����εĵ����ź�
		if (CS_COND_BROADCAST == m_state)	
			return 0;

		m_state = CS_COND_SINGLE;

		return SetEvent(m_cond);
#else
		return pthread_cond_signal(&m_cond);
#endif // _WIN32
	}


	int CondBroadCast()
	{
#ifdef _WIN32
		m_state = CS_COND_BROADCAST;
		return SetEvent(m_cond);
#else
		return pthread_cond_broadcast(&m_cond);
#endif // _WIN32

	}

private:
	cond_t m_cond;

	Mutex& m_mutex;							//�ⲿ�������������������

#ifdef _WIN32
	std::atomic<int> m_waitThreadNum;		//�ȴ��������������߳�����

	std::atomic<short> m_state;
#endif // _WIN32
};