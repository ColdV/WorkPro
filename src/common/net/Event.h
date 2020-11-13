#pragma once


#include "stdafx.h"
#include <map>
#include <set>
#include <unordered_set>
#include <functional>
#include <memory>
#include "Poller.h"
#include "Buffer.h"
#include "IOCP.h"
#include "thread/Mutex.h"

enum
{
	EV_IOREAD = 1,
	EV_IOWRITE = 1 << 1,
	EV_IOEXCEPT = 1 << 2,
	EV_TIMEOUT = 1 << 3,
	EV_SIGNAL = 1 << 4,
	EV_CANCEL = 1 << 5,
	EV_ERROR = 1 << 6,
};


enum
{
	EV_CTL_ADD = 1,
	EV_CTL_DEL,
};


namespace chaos
{
	inline int GetLastErrorno()
	{
#ifdef _WIN32
		return WSAGetLastError();
#else
		return errno;
#endif // _WIN32

	}
}


namespace chaos
{
	class Event;
	class EventCentre;
	class Listener;
	class Connecter;
	class Timer;


	union EventKey
	{
		socket_t	fd;
		timer_id	timerId;
	};

	const int IO_CARE_EVENT = EV_IOREAD | EV_IOWRITE | EV_IOEXCEPT | EV_CANCEL;
	const int TIMEOUT_CART_EVENT = EV_TIMEOUT | EV_CANCEL;

	//�¼�
	class Event : public NonCopyable
	{
	public:
		friend class EventCentre;
		friend class Poller;

		typedef std::function<void(Event* pEv, short ev, void* userdata)> EventCallback;

		typedef std::function<void(Event* pEv, int errcode, void* userdata)> EventErrCallback;

		typedef std::function<void(int ret)> EventRegisterCallback;

		virtual ~Event() {}

		//��Ӧ�¼�����
		virtual void Handle() = 0;

		short GetEv() const { return m_ev; }

		short GetCurEv() { if (m_curEv.empty()) return 0; return m_curEv.front(); }

		const EventKey& GetEvKey() const { return m_evKey; }

		EventCentre* GetCentre() const { return m_pCenter; }

		void CancelEvent();

		void SetEventCallback(const EventCallback& cb, void* userdata) { m_callback = cb; m_userdata = userdata; }

	protected:
		Event(short ev, const EventKey& evKey);

		void SetEv(short ev) { m_ev = ev; }

		void UpdateEvent(short op, short ev);

		void CallErr(int errcode);

		void SetRegisterCallback(const EventRegisterCallback& cb) { m_registerCb = cb; }

	private:
		void Callback() { if (m_callback) m_callback(this, GetCurEv(), m_userdata); }

		void PushCurEv(short ev) { m_curEv.push(ev); }

		void PopCurEv() { if (!m_curEv.empty()) m_curEv.pop(); }

		void SetCenter(EventCentre* pCentre) { m_pCenter = pCentre; }

		void SetEvKey(const EventKey& evKey) { memcpy(&m_evKey, &evKey, sizeof(evKey)); }

		void CallbackRegister(int ret) { if (m_registerCb) m_registerCb(ret); }

	private:
		EventCentre* m_pCenter;		//�������¼�����

		short m_ev;					//ע����¼�
		
		std::queue<short> m_curEv;	//��ǰ�������¼�����

		EventKey	m_evKey;

		EventCallback m_callback;

		void* m_userdata;

		EventRegisterCallback m_registerCb;
	};


	//�¼�����(�¼���Ӧ,�ַ�)
	class EventCentre : public NonCopyable
	{
	public:
		struct ActiveEvent
		{
			short ev;
			Event* pEvent;
		};


		typedef std::map<socket_t, Event*> NetEventMap;
		typedef std::map<int, Event*>	SignalEventMap;
		typedef std::list<Event*>	ActiveEventList;
		typedef std::queue<Event*>	EvQueue;
		typedef std::vector<Event*> EventList;
		
	public:
		EventCentre();
		~EventCentre();

		int Init();

		int EventLoop(int loopTickTimeMs = 0);

		void Exit() { m_running = false; }

		int RegisterEvent(Event* pEvent);

		void PushEvent(Event* pEvent, short ev);

		//�����¼�
		//@ev:��Ҫ���µ��¼�
		void UpdateEvent(Event* pEvent, short op, short ev);

		Mutex& GetMutex() { return m_mutex; }

	private:
		int CancelEvent(Event* pEvent);

		int ProcessActiveEvent();

		//�������ע���¼��ͻ�¼�
		void ClearAllEvent();
	
		//���㵱ǰ�ȴ�IO��timeout
		int CalculateTimeout();

	private:
		Poller* m_pPoller;				//�����¼�������

		Timer* m_pTimer;				//��ʱ��

		SignalEventMap m_signalEvs;

		EventList m_activeEvs;			//��¼�

		EventList m_waittingEvs;			//�ѵȴ��е��¼�

		bool m_running;

		int m_evcount;

		bool m_isInit;

		Mutex m_mutex;
	};



	class Listener :public Event
	{
	public:
#ifdef IOCP_ENABLE
		static const int INIT_ASYNACCEPTING			= 8;
		static const int INIT_ACCEPTADDRBUF_SIZE	= 128; //AcceptEx�����buffer��С(����Э�������ַ��С + 16) * 2
#endif // IOCP_ENABLE

		typedef std::function<void(Listener* listener, Connecter* newconn, void* userdata)>	ListenerCb;

		explicit Listener(socket_t fd);

		~Listener();

		static Listener* CreateListener(int af, int socktype, int protocol, unsigned short port, const char* ip = 0);

		int Listen(const sockaddr* sa, int salen);

		Socket& GetSocket() { return *m_socket; }

		void SetListenerCb(const ListenerCb& cb, void* pCbData) { m_cb = cb; m_userdata = pCbData; }

		void CallListenerCb(Connecter* newconn) { if (m_cb) m_cb(this, newconn, m_userdata); }

	protected:
#ifdef IOCP_ENABLE
		int StartAsynRequest();
#endif // IOCP_ENABLE

	private:
		virtual void Handle() override;

#ifdef IOCP_ENABLE
		int AsynAccept(LPACCEPT_OVERLAPPED lo);

		//GetQueuedCompletionStatus��Ļص�
		void AcceptComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk);
#endif // IOCP_ENABLE

		void RegisterCallback(int ret);

		void DoneAccept(socket_t acceptedfd);

	private:
		Socket* m_socket;

		ListenerCb m_cb;

		void* m_userdata;

#ifdef IOCP_ENABLE
		LPACCEPT_OVERLAPPED m_acceptOls;

		int* m_overlappedsRefCnt;

		char** m_acceptBuffers;

		std::queue<LPACCEPT_OVERLAPPED> m_acceptedq;
#endif // IOCP_ENABLE
	};


	class Connecter : public Event
	{
	public:
		typedef std::function<void(Connecter* pConnect, int nTransBytes, void* userdata)> NetCallback;

		union SockAddr
		{
			sockaddr sa;
			sockaddr_in sin;
			sockaddr_in6 sin6;
		};

		explicit Connecter(socket_t fd);

		~Connecter();

		Socket& GetSocket() { return *m_socket; }

		int Connect(sockaddr* sa, int salen);

		//������д�뵽buff��, ��������һ��EV_IOWRITE�¼�����һ֡д��socket
		int WriteBuffer(const char* buf, int len);

		//д��socket
		int Send(const char* buf, int size);

		//��RBuffer�ж�ȡlen���ֽڵ�buf��
		//return:����ʵ�ʶ�ȡ���ֽ���
		int ReadBuffer(char* buf, int size);

		//��ȡRBuffer�пɶ�ȡ���ֽ���
		int GetReadableSize() { if (!m_pReadBuffer) return 0; return m_pReadBuffer->GetReadSize(); }

		void SetCallback(const NetCallback& readcb, const NetCallback& writecb, const NetCallback& connectcb, void* userdata);

		//����һ�����¼�
		void EnableEvent(short ev);

		//�رչ�ע���¼�
		void DisableEvent(short ev);

	private:
		virtual void Handle() override;

		void RegisterCallback(int ret);

		void CallbackRead(int nTransferBytes) { if (m_readcb) m_readcb(this, nTransferBytes, m_userdata); }

		void CallbackWrite(int nTransferBytes) { if (m_writecb) m_writecb(this, nTransferBytes, m_userdata); }

		void CallbackConnect(bool bOk) { if (m_connectcb) m_connectcb(this, bOk, m_userdata); }

		int HandleRead();

		int HandleWrite();

#ifdef IOCP_ENABLE
		int AsynRead();

		int AsynWrite();

		void ReadComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk);

		void WriteComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk);

		void ConnectComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool bOk);
#endif // IOCP_ENABLE

	private:
		Socket* m_socket;

		Buffer* m_pReadBuffer;

		Buffer* m_pWriteBuffer;

#ifdef IOCP_ENABLE
		LPCOMPLETION_OVERLAPPED m_pReadOverlapped;
		bool m_isPostRecv;			//�Ƿ���Ͷ��WSARecv�¼�(IOCP�ڵȴ��¼����)

		LPCOMPLETION_OVERLAPPED m_pWriteOverlapped;
		bool m_isPostWrite;			//�Ƿ���Ͷ��WSASend�¼�(IOCP�ڵȴ��¼����)

		LPCOMPLETION_OVERLAPPED m_pConnectOverlapped;
		bool m_isPostConnect;		//�Ƿ���Ͷ��ConnectEx�¼�(IOCP�ڵȴ��¼����)
		//std::shared_ptr<COMPLETION_OVERLAPPED> m_pConnectOverlapped;

#endif // IOCP_ENABLE

		NetCallback m_readcb;

		NetCallback m_writecb;

		NetCallback m_connectcb;

		void* m_userdata;

		SockAddr m_peeraddr;

		socklen_t m_peeraddrlen;
	};



	class TimerEvent : public Event
	{
	public:
		friend class Timer;
		typedef std::function<void()> TimerHandler;

		TimerEvent(timer_id timerId, uint32 timeout, bool isLoop = false) :
			Event(EV_TIMEOUT, (EventKey&)timerId),
			m_timeout(timeout),
			m_isLoop(isLoop),
			m_isCancel(false),
			m_isSuspend(false),
			m_handleFunc(NULL)
		{
			m_nextTime = time(NULL) + m_timeout;
		}

		virtual ~TimerEvent()
		{}

		uint32 GetTimeOut() const { return m_timeout; }

		time_t GetNextTime() const { return m_nextTime; }

		bool IsLoop() const { return m_isLoop; }
		void SetLoop(bool isLoop) { m_isLoop = isLoop; }

		void Cancel() { m_isCancel = true; };

		void Suspend() { m_isSuspend = true; };

		void Resume() { m_isSuspend = false; }

		bool IsCancel() const { return m_isCancel; }

		bool IsSuspend() const { return m_isSuspend; }

		void SetTimerHandle(const TimerHandler& func) { m_handleFunc = func; };

	private:
		virtual void Handle() override;

		void SetNextTime() { m_nextTime = time(NULL) + m_timeout; }

		void DefaultHandle();

	private:
		uint32 m_timeout;

		time_t m_nextTime;

		bool m_isLoop;						//ѭ����ʱ��

		bool m_isCancel;					//ȡ����ʱ��

		bool m_isSuspend;					//��ͣ��ʱ��

		TimerHandler m_handleFunc;
	};


}	//namespace chaos