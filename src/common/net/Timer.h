#pragma once

#include <set>
#include <unordered_map>
#include "template/MinHeap.h"
#include "Event.h"


namespace chaos
{ 

	struct TimerCmp
	{
		bool operator()(const std::weak_ptr<TimerEvent>& weakl, const std::weak_ptr<TimerEvent>& weakr) const
		{
			std::shared_ptr<TimerEvent> l(weakl.lock());
			std::shared_ptr<TimerEvent> r(weakr.lock());
			if (!l || !r)
				return false;

			return l->GetNextTime() < r->GetNextTime();
		}

	};

	class Timer
	{
	public:
		static const int INIT_ID_SIZE = 128;
		static const int BYTE2BIT = 8;

		friend EventCentre;
		friend TimerEvent;

		typedef std::unordered_map<int, std::shared_ptr<TimerEvent>> TimerMap;
		typedef std::shared_ptr<TimerEvent> TimerSharedPtr;
		typedef std::weak_ptr<TimerEvent> TimerWeakPtr;

		Timer(EventCentre* pCentre);
		~Timer();

		void Launch(EventList& activeEvents);

		//��Ӷ�ʱ��,���ض�ʱ��ID
		uint32 AddTimer(const EventSharedPtr& pEvent) { return AddTimer(std::static_pointer_cast<TimerEvent>(pEvent)); }

		uint32 AddTimer(const TimerSharedPtr& pTimerEv);

		uint32 DelTimer(TimerEvent* pTimerEv);

		//��ȡ��һ�γ�ʱʱ��
		//-1:��ǰû�ж�ʱ��, 0:��ǰ���о����Ķ�ʱ��, >0:�¸���ʱ����ʱʱ��
		int GetNextTimeout();

		int Size() { return m_timers.Size(); }

		void Clear();

		//����һ����ʱ��ID, ����0��ʾ��ID����
		static timer_id CreateTimerID();

		static void ReleaseTimerID(timer_id id);

	private:
		TimerMap& GetAllTimer() { return m_timerMap; }

	private:
		MinHeap<TimerWeakPtr, TimerCmp> m_timers;

		EventCentre* m_pCentre;

		TimerMap m_timerMap;

		TIME_T	m_lastRunTime;

	private:
		static std::vector<byte> s_ids;

		static std::atomic<uint32> s_curTimers;			//��ʱ������

		static Mutex s_mutex;
	};

}
