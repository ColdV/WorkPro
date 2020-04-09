#pragma once

struct Event;

typedef void (*EventCallback)(Event* pEvent, void* userData);

#include "../common/stdafx.h"
#include <map>
#include <set>

enum
{
	EV_IOREAD = 1,
	EV_IOWRITE = 1 << 1,
	EV_IOEXCEPT = 1 << 2,
	EV_TIMEOUT = 1 << 3,
};

struct TimerEvent
{
	unsigned int timerID;
	int timeOut;
	//TimerCallback cb;
};


struct SocketEvent		//need scoketEvMgr
{
	unsigned int fd;
	int size;
	int totalSize;
	char* buffer;
};


struct Event
{
	int evID;
	int ev;

	union
	{
		TimerEvent		evTimer;
		SocketEvent		evSocket;
	} Ev;

	EventCallback	evCb;
	bool isLoop;
	void* userData;
};


class EventNew
{
public:
	virtual ~EventNew() = 0;

protected:
	EventNew() {};

	EventNew* NewEvent(short ev, EventCallback evCb, bool isLoop) {};

protected:
	unsigned int m_id;
	short m_ev;
	EventCallback m_evCb;
	bool m_isLoop;

	static unsigned int m_increment;
	static std::set<unsigned int> m_readyId;
};


namespace EventFrame
{
	class EventHandler;
	class Event;
	class Socket;

	typedef std::map<Event*, EventHandler*> EventMap;


	class Socket
	{
	};


	//�����¼�(��Դ��)
	class Event
	{
	public:
		Event() {}
		virtual ~Event() = 0;

		void SetID(uint32 id) { m_id = id; }
		uint32 GetID() { return m_id; }

		void SetLoop(bool isLoop) { m_isLoop = isLoop; }
		bool IsLoop() { return m_isLoop; }

		void SetEv(uint32 ev) { m_ev = ev; }
		uint32 GetEv() { return m_ev; }

	private:
		uint32 m_id;
		bool m_isLoop;
		uint32 m_ev;
		//EventHandler* m_handler;
	};


	////�¼�������
	//class EventLauncher
	//{
	//public:
	//	EventLauncher() {}
	//	virtual ~EventLauncher() = 0;

	//	virtual void Run() = 0;
	//};


	class IOEventDispatcher;

	//�¼���ע�ᡢ���١��ַ�
	class EventCentre
	{
	public:
		EventCentre() {}
		~EventCentre() {}

		int Init();

		void EventLoop();

		int RegisterEvent(Event* ev, EventHandler* pHandler, bool isReady);

		int CancelEvent(Event* ev);

		int DispatchEvent();

		int IOEventDispatch();

		int SignalDispatch();

		int TimerDispatch();

		int ProcessReadyEvent();

	private:
		EventMap m_netEvs;			//IOMasterEvent->AllIOEvent
		//EventHandler* m_pIOHandler;
		IOEventDispatcher* m_pIOHandler;

		EventMap m_timerEvs;
		//EventHandler* m_pTimerHandler;

		EventMap m_signalEvs;

		//EventMap m_events;
		EventMap m_readyEv;
	};



	//�����¼�������
	class EventHandler
	{
	public:
		EventHandler() {}
		virtual ~EventHandler() = 0;

		virtual void Handle(Event* pEv) = 0;
	};


	//�����¼�������
	class NetEventDispatcher
	{
	public:
		NetEventDispatcher();
		virtual ~NetEventDispatcher() = 0;

		virtual void Init() = 0;
	};


	class NetEvent : public Event
	{
	public:
		NetEvent(Socket* pSocket) { pSocket = m_pSocket; }
		~NetEvent()
		{
			if (m_pSocket)
				delete m_pSocket;
		}

	private:
		Socket* m_pSocket;
	};


	class NetEventHandler : public EventHandler
	{
	public:
		NetEventHandler();
		~NetEventHandler();

		virtual void Handle(Event* pEv);
	};

}	//namespace EventFrame