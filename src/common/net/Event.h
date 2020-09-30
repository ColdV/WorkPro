#pragma once


#include "../../common/stdafx.h"
#include <map>
#include <set>
#include <unordered_set>
#include <functional>
#include "Poller.h"
#include "Buffer.h"
#include "IOCP.h"
#include "../thread/Mutex.h"

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
		int			signal;
	};


	//�¼�
	class Event : public NonCopyable
	{
	public:
		friend class EventCentre;
		friend class Poller;

		typedef std::function<void(Event* pEv, short ev, void* userdata)> EventCallback;

		typedef std::function<void(Event* pEv, int errcode, void* userdata)> EventErrCallback;

		typedef std::function<void(int ret)> EventRegisterCallback;

		virtual ~Event()
		{
			//CancelEvent();
		}

		virtual void Handle() = 0;

		short GetEv() const { return m_ev; }

		void SetEv(short ev) { m_ev = ev; }

		short GetCurEv() { if (m_curEv.empty()) return 0; return m_curEv.front(); }

		const EventKey& GetEvKey() const { return m_evKey; }

		EventCentre* GetCentre() const { return m_pCenter; }

		void CancelEvent();

		void SetEventCallback(const EventCallback& cb, void* userdata) { m_callback = cb; m_userdata = userdata; }

		//void SetErrCallback(const EventErrCallback& cb, void* userdata) { m_errCallback = cb, m_errUserdata = userdata; }

	protected:
		Event(short ev, const EventKey& evKey);
		Event();

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


	//�¼���ע�ᡢ���١��ַ�
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
		typedef std::vector<Event*> ActiveEventAry;
		
	public:
		EventCentre();
		~EventCentre();

		int Init();

		int EventLoop(int loopTickTimeMs = 0);

		void Exit() { m_running = false; }

		int RegisterEvent(Event* pEvent);

		int CancelEvent(Event* pEvent);

		void PushActiveEv(Event* pEvent, short ev);

		Mutex& GetMutex() { return m_mutex; }

	private:
		void PushActiveEv(Event* pEvent);

		int SignalDispatch();

		int ProcessActiveEvent();

		//�������ע���¼��ͻ�¼�
		void ClearAllEvent();

	private:
		Poller* m_pPoller;				//�����¼�������

		Timer* m_pTimer;				//��ʱ��

		SignalEventMap m_signalEvs;

		EvQueue m_activeEvs;

		bool m_running;

		int m_evcount;

		bool m_isInit;

		Mutex m_mutex;
	};



	class Listener :public Event
	{
	public:
#ifdef IOCP_ENABLE
		static const int INIT_ASYNACCEPTING = 8;
		static const int INIT_ACCEPTADDRBUF = 256;
#endif // IOCP_ENABLE

		typedef std::function<void(Listener* listener, Connecter* newconn, void* userdata)>	ListenerCb;

		Listener(socket_t fd);

		~Listener();

		static Listener* CreateListener(int af, int socktype, int protocol, unsigned short port, const char* ip = 0);

		int Listen(const sockaddr* sa, int salen);

		virtual void Handle() override;

		Socket& GetSocket() { return *m_socket; }

		void SetListenerCb(const ListenerCb& cb, void* pCbData) { m_cb = cb; m_userdata = pCbData; }

		void CallListenerCb(Connecter* newconn) { if (m_cb) m_cb(this, newconn, m_userdata); }

	protected:
#ifdef IOCP_ENABLE
		int StartAsynRequest();
#endif // IOCP_ENABLE

	private:
#ifdef IOCP_ENABLE
		int AsynAccept(LPACCEPT_OVERLAPPED lo);

		//GetQueuedCompletionStatus��Ļص�
		void AcceptComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool ok);
#endif // IOCP_ENABLE

		void RegisterCallback(int ret);

	private:
		Socket* m_socket;

		ListenerCb m_cb;

		void* m_userdata;

#ifdef IOCP_ENABLE
		LPACCEPT_OVERLAPPED m_acceptOls;

		int* m_overlappedsRefCnt;

		Buffer* m_acceptBuffers;

		std::queue<LPACCEPT_OVERLAPPED> m_acceptedq;
#endif // _WIN32
	};


	class Connecter : public Event
	{
	public:
		typedef std::function<void(Connecter* pConnect, int nTransBytes, void* userdata)> NetCallback;

		Connecter(socket_t fd);

		~Connecter();

		virtual void Handle() override;

		Socket& GetSocket() { return *m_socket; }

		//������д�뵽buff��, ��������һ��EV_IOWRITE�¼�����һ֡д��socket
		int WriteBuffer(const char* buf, int len);

		//д��socket
		int Write(const char* buf, int size);

		//��RBuffer�ж�ȡlen���ֽڵ�buf��
		//return:����ʵ�ʶ�ȡ���ֽ���
		int ReadBuffer(char* buf, int size);

		//��ȡRBuffer�пɶ�ȡ���ֽ���
		int GetReadableSize() { if (!m_pRBuffer) return 0; return m_pRBuffer->GetReadSize(); }

		void SetCallback(const NetCallback& readcb, void* readCbArg, const NetCallback& writecb, void* writeCbArg);

	private:
		void RegisterCallback(int ret);

		void CallbackRead(int nTransferBytes) { if (m_readcb) m_readcb(this, nTransferBytes, m_readCbArg); }

		void CallbackWrite(int nTransferBytes) { if (m_writecb) m_writecb(this, nTransferBytes, m_writeCbArg); }

		int HandleRead();

		int HandleWrite();

#ifdef IOCP_ENABLE
		int AsynRead();

		int AsynWrite();

		void ReadComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool ok);

		void WriteComplete(OVERLAPPED* o, DWORD bytes, ULONG_PTR lpCompletionKey, bool ok);
#endif // IOCP_ENABLE

	private:
		Socket* m_socket;

		Buffer* m_pRBuffer;

		Buffer* m_pWBuffer;

#ifdef IOCP_ENABLE
		LPCOMPLETION_OVERLAPPED m_pROverlapped;

		bool m_isPostRecv;

		LPCOMPLETION_OVERLAPPED m_pWOverlapped;

		bool m_isPostWrite;
#endif // IOCP_ENABLE

		NetCallback m_readcb;

		void* m_readCbArg;

		NetCallback m_writecb;

		void* m_writeCbArg;
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

		virtual void Handle() override;

		void Cancel() { m_isCancel = true; };

		void Suspend() { m_isSuspend = true; };

		void Resume() { m_isSuspend = false; }

		bool IsCancel() const { return m_isCancel; }

		bool IsSuspend() const { return m_isSuspend; }

		void SetTimerHandle(const TimerHandler& func) { m_handleFunc = func; };

	private:
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