/************C++ Header File****************
#
#	Filename: Epoll.h
#
#	Author: H`W
#	Description: ---
#	Create: 2018-08-10 10:12:31
#	Last Modified: 2018-08-10 10:12:31
*******************************************/

#ifndef _WIN32

#pragma once

#include "NetDrive.h"
#include <sys/epoll.h>

namespace NetFrame
{

	class Epoll : public NetDrive
	{
	public:
		enum
		{
			MAX_FD = 1024,
		};

		static Epoll& Instance();

		virtual ~Epoll() {}

		virtual int InitIO(/*const char* ip, int port, uint32 max_fd*/);

		virtual int Launch();

	protected:
		Epoll();

		virtual void RegistFd(socket_t fd, short ev);

		virtual void CancelFd(socket_t fd, short ev);

	private:
		int m_max_fd;
		int m_epfd;
		epoll_event* m_events;
	};

}

#endif // !_WIN32
