#pragma once

#include "../../common/stdafx.h"
#include "Thread.h"
#include "ThreadTask.h"
#include "Mutex.h"
#include "Sem.h"
#include "Condition.h"


class ThreadPool : public NonCopyable
{
public:
	//���ֵ�������߳��еȴ��źŵĳ�ʱʱ��,��Ҫ���������ȴ�
	//������stop��ʱ�򷢳�BroadCast��ʱ��,
	//�����߳̿��ܴ�m_running�ж�֮��͵ȴ��ź�֮ǰ
	//��ֹ�̳߳�ʵ����ֹͣ,���������ڵȴ���
	static const int THREAD_WAIT_TIMEOUT = 10;

	explicit ThreadPool(int nThreadNum = -1);

	~ThreadPool();

	int Run();

	int Stop();

	int PushTask(ThreadTask* pTask);
	
private:
	static THREAD_FUNCTION_PRE PoolWorkFunc(void*);

private:
	std::list<Thread*> m_threads;

	std::queue<ThreadTask*> m_tq;		//�������

	uint32 m_threadNum;

	Mutex m_mutex;

	Condition m_cond;

	bool m_running;

};

