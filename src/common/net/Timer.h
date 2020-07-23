#pragma once


#include "../../common/template/MinHeap.h"
#include <set>
#include <unordered_map>
#include "Event.h"


namespace chaos
{ 

	struct TimerCmp
	{
		bool operator()(const TimerEvent* l, const TimerEvent* r) const
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

		typedef std::unordered_map<int, TimerEvent*> TimerMap;

		Timer();
		~Timer();

		void DispatchTimer();

		//��Ӷ�ʱ��,���ض�ʱ��ID
		uint32 AddTimer(Event* pTimerEv) { return AddTimer((TimerEvent*)pTimerEv); }

		uint32 AddTimer(TimerEvent* pTimerEv);

		uint32 DelTimer(TimerEvent* pTimerEv);

		int Size() { return m_timers.Size(); }

		int TotalSize() { return m_timers.TotalSize(); }


	public:
		//����һ����ʱ��ID, ����0��ʾ��ID����
		static timer_id CreateTimerID();

	private:

		//��չID
		static int ExpandID();

	private:
		MinHeap<TimerEvent*, TimerCmp> m_timers;
		TimerMap m_timerMap;
		std::set<unsigned int>	m_delList;
		std::set<unsigned int>	m_deled;
		time_t	m_lastRunTime;

	private:
		static char* s_ids;
		static uint32 s_curTimers;
		static timer_id s_maxIDSize;
	};

}
