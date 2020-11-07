#pragma once


#include "template/MinHeap.h"
#include <set>
#include <unordered_map>
#include "Event.h"


namespace chaos
{ 

	struct TimerCmp
	{
		bool operator()(const TimerEvent* l, const TimerEvent* r) const
		{
			return l->GetNextTime() < r->GetNextTime();
		}

	};

	class Timer
	{
	public:
		enum 
		{
			INIT_ID_SIZE = 128,
		};

		friend EventCentre;

		typedef std::unordered_map<int, TimerEvent*> TimerMap;
		typedef std::vector<Event*> EventList;

		//Timer& Instance();
		Timer();
		~Timer();

		void DispatchTimer(EventList& activeEvents);

		//��Ӷ�ʱ��,���ض�ʱ��ID
		uint32 AddTimer(Event* pTimerEv) { return AddTimer((TimerEvent*)pTimerEv); }

		uint32 AddTimer(TimerEvent* pTimerEv);

		uint32 DelTimer(TimerEvent* pTimerEv);

		int Size() { return m_timers.Size(); }

		void Clear();

	public:
		//����һ����ʱ��ID, ����0��ʾ��ID����
		static timer_id CreateTimerID();

	private:
		TimerMap& GetAllTimer() { return m_timerMap; }

		//��չID
		static int ExpandID();

	private:
		MinHeap<TimerEvent*, TimerCmp> m_timers;

		TimerMap m_timerMap;

		time_t	m_lastRunTime;

	private:
		static char* s_ids;

		static uint32 s_curTimers;			//��ʱ������

		static timer_id s_maxIDSize;
	};

}
