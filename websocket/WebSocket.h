#pragma once

#include <string>
#include <iostream>
#include <sstream>

#ifdef _WIN32
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")
#endif // _WIN32


#define MAGICKEY	"258EAFA5-E914-47DA-95CA-C5AB0DC85B11"			//��websocket key���� ʹ�õĹ̶��ַ���

enum WS_Status
{
	WS_STATUS_CONNECT = 0,
	WS_STATUS_UNCONNECT = 1,
};

enum WS_FrameType
{
	WS_EMPTY_FRAME = 0xF0,
	WS_ERROR_FRAME = 0xF1,
	WS_TEXT_FRAME = 0x01,
	WS_BINARY_FRAME = 0x02,
	WS_PING_FRAME = 0x09,
	WS_PONG_FRAME = 0x0A,
	WS_OPENING_FRAME = 0xF3,
	WS_CLOSING_FRAME = 0x08,
};

class WebSokcet
{
public:
	WebSokcet();
	virtual ~WebSokcet();

	//����
	int Handshake(std::string &request, std::string &response);

	//����
	int Decode(std::string inFrame, std::string &outMessage);

	//����
	int Encode(std::string inMessage, std::string &outFrame, enum WS_FrameType frameType);

	//״̬��ȡ-����
	int GetStatus() const { return m_status; }
	void SetStatus(WS_Status eStatus) { m_status = eStatus; }

private:
	int m_status;
};