#pragma once


#include"MinHeap.h"
#include <set>
#include "Event.h"

//typedef void (*TimerCallback)(TimerEvent* pEvent, void* userData);
//
//struct TimerEvent
//{
//	unsigned int timerID;
//	int timeOut;
//	TimerCallback cb;
//};

namespace NetFrame
{ 

	struct TimerCmp
	{
		bool operator()(TimerEvent* l, TimerEvent* r) const
		{
			return l->GetTimeOut() < r->GetTimeOut();
		}

	};

	class Timer	//:public MinHeap<TimerEvent, TimerCmp>  ÿ�ε���λ�ú� ����TimerEvent�е�POS�ֶ�
	{
	public:
		enum 
		{
			INIT_ID_SIZE = 128,
		};

		Timer();
		~Timer();

		void DispatchTimer();

		//int AddTimer(Event* ev, int timeOut, EventCallback cb);

		//int AddTimer(Event* ev, unsigned int hour, unsigned int min, unsigned int sec, EventCallback cb);

		//��Ӷ�ʱ��,���ض�ʱ��ID
		uint32 AddTimer(TimerEvent* pTimerEv);

		//ɾ��:��timer����dellistɾ���б��� ÿ��ִ�ж�ʱ��ʱ ��⵱ǰ��ʱ�������ɾ���б��� ��ɾ��
		//int DelTimer(Event* ev);

		//int DelTimer(unsigned int timerID);

		uint32 DelTimer(TimerEvent* pTimerEv);

		//void StartTimer();
	
		//void StopTimer();

		//Event* PopTimer() { return TopTimer(); }

		//Event* TopTimer() { return m_timers.Top(); }

		int GetSize() { return m_timers.GetSize(); }

		int GetTotalSize() { return m_timers.GetTotalSize(); }

	private:
		//uint32 NewID() { if (m_maxID >= 0xFFFFFFFF)return 0; return ++m_maxID; }

		//TimerEvent* PopTimer() { return TopTimer(); }

		//TimerEvent* TopTimer() { return *m_timers.Front(); }

		//����һ����ʱ��ID, ����0��ʾ��ID����
		uint32 AllocaTimerID();

		//��չID
		int ExpandID();

	private:
		MinHeap<TimerEvent*, TimerCmp> m_timers;
		//std::set<unsigned int>	m_timerIDs;
		std::set<unsigned int>	m_delList;
		std::set<unsigned int>	m_deled;
		char* m_ids;
		uint32 m_maxIDSize;
		time_t	m_lastRunTime;
	};

}
