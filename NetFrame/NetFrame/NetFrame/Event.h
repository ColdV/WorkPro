#pragma once


#include "../../common/stdafx.h"
#include <map>
#include <set>
#include "NetDrive.h"
#include "Buffer.h"

enum
{
	EV_IOREAD = 1,
	EV_IOWRITE = 1 << 1,
	EV_IOEXCEPT = 1 << 2,
	EV_TIMEOUT = 1 << 3,
	EV_SIGNAL = 1 << 4,
};


namespace NetFrame
{
	//class EventHandler;
	class Event;
	class EventCentre;
	class Timer;


	union EventKey
	{
		socket_t	fd;
		timer_id	timerId;
		int			signal;
	};


	struct EvAndKey
	{
		uint32 ev;
		EventKey key;
	};

	//�����¼�(��Դ��)
	class Event
	{
	public:
		virtual void Handle() = 0;

		uint32 GetEv() const { return m_ev; }

		uint32 GetCurEv() const { return m_curEv; }

		void SetCurEv(uint32 ev) { m_curEv = ev; }

		const EventKey* GetEvKey() const { return m_pEvKey; }

		EventCentre* GetCentre() const { return m_pCenter; }

	protected:
		Event(EventCentre* pCentre, uint32 ev, EventKey* pEvKey) :
			m_ev(ev),
			m_curEv(0),
			m_pEvKey(pEvKey),
			m_pCenter(pCentre)
		{
		}

		virtual ~Event() = 0
		{
			if (m_pEvKey)
				delete m_pEvKey;
		}

	private:
		EventCentre* m_pCenter;		//�������¼�����
		uint32 m_ev;		//ע����¼�
		uint32 m_curEv;		//��ǰ�������¼�
		EventKey*	m_pEvKey;
	};


	//�¼���ע�ᡢ���١��ַ�
	class EventCentre
	{
	public:
		typedef std::map<socket_t, Event*> NetEventMap;
		typedef std::map<int, Event*>	TimerEventMap;
		typedef std::map<int, Event*>	SignalEventMap;
		typedef std::list<Event*>	ActiveEventList;


	public:
		EventCentre();
		~EventCentre();

		int Init();

		void EventLoop();

		int RegisterEvent(Event* ev);

		int CancelEvent(Event* ev);

		int DispatchEvent();

		void PushActiveEv(Event* ev) { m_activeEvs.push_back(ev); }

	private:
		int NetEventDispatch();

		int SignalDispatch();

		int TimerDispatch();

		int ProcessActiveEvent();

	private:
		NetEventMap m_netEvs;			//IOMasterEvent->AllIOEvent

		NetDrive* m_pNetDrive;

		TimerEventMap m_timerEvs;

		Timer* m_pTimer;

		SignalEventMap m_signalEvs;

		ActiveEventList m_activeEvs;
	};




	class NetEvent : public Event
	{
	public:
		NetEvent(EventCentre* pCentre, Socket* pSocket, uint32 ev, EventKey* pEvKey):
			Event(pCentre, ev, pEvKey),
			m_pSocket(pSocket)
		{
			m_pRBuffer = new Buffer;
			m_pWBuffer = new Buffer;
		}

		~NetEvent()
		{
			if (m_pSocket)
				delete m_pSocket;

			if (m_pRBuffer)
				delete m_pRBuffer;

			if (m_pWBuffer)
				delete m_pWBuffer;
		}

		Socket* GetSocket() const { return m_pSocket; }

		virtual void Handle() override;

	private:
		int HandleListen();

		int HandleRead();

		int HandleWrite();

	private:
		Socket* m_pSocket;
		Buffer* m_pRBuffer;
		Buffer* m_pWBuffer;
	};


	class TimerEvent : public Event
	{
	public:
		TimerEvent(EventCentre* pCentre, uint32 ev, EventKey* pEvKey, uint32 timeOut, bool isLoop = false) :
			Event(pCentre, ev, pEvKey),
			m_timeOut(timeOut),
			m_isLoop(isLoop)
		{
		}

		virtual ~TimerEvent()
		{}

		uint32 GetTimeOut() const { return m_timeOut; }

		bool IsLoop() const { return m_isLoop; }
		void SetLoop(bool isLoop) { m_isLoop = isLoop; }

		virtual void Handle() override;

	private:
		uint32 m_timeOut;
		bool m_isLoop;
	};


}	//namespace NetFrame