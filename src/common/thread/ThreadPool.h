#pragma once

#include <functional>
#include <atomic>
#include <memory>
#include "stdafx.h"
#include "Thread.h"
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

	typedef std::function<void()>	ThreadTask;

	explicit ThreadPool(int nThreadNum = -1);

	~ThreadPool();

	int Run();

	int Stop();

	void PushTask(const ThreadTask& task);
	
private:
	static THREAD_FUNCTION_PRE PoolWorkFunc(void*);

private:
	std::vector<std::unique_ptr<Thread>> m_threads;

	std::queue<ThreadTask> m_tq;

	uint32 m_threadNum;

	Mutex m_mutex;

	Condition m_cond; 

	std::atomic<bool> m_running;

};

