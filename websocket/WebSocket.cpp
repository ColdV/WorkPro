#include "WebSocket.h"
#include "sha1.h"
#include "base64.h"


WebSokcet::WebSokcet()
{
	m_status = WS_STATUS_UNCONNECT;
}

WebSokcet::~WebSokcet()
{
}


//����
int WebSokcet::Handshake(std::string &request, std::string &response)
{
	// ����http����ͷ��Ϣ
	int ret = WS_STATUS_UNCONNECT;
	std::istringstream stream(request.c_str());
	std::string reqType;
	std::getline(stream, reqType);
	if (reqType.substr(0, 4) != "GET ")
	{
		return ret;
	}

	std::string header;
	std::string::size_type pos = 0;
	std::string websocketKey;
	while (std::getline(stream, header) && header != "\r")
	{
		header.erase(header.end() - 1);
		pos = header.find(": ", 0);
		if (pos != std::string::npos)
		{
			std::string key = header.substr(0, pos);
			std::string value = header.substr(pos + 2);
			if (key == "Sec-WebSocket-Key")
			{
				ret = WS_STATUS_CONNECT;
				websocketKey = value;
				break;
			}
		}
	}

	if (ret != WS_STATUS_CONNECT)
	{
		return ret;
	}

	// ���http��Ӧͷ��Ϣ
	response = "HTTP/1.1 101 Switching Protocols\r\n";
	response += "Upgrade: websocket\r\n";
	response += "Connection: upgrade\r\n";
	response += "Sec-WebSocket-Accept: ";

	//const std::string magicKey("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	const std::string magicKey(MAGICKEY);
	std::string serverKey = websocketKey + magicKey;

	char shaHash[32];
	memset(shaHash, 0, sizeof(shaHash));
	//sha1::calc(serverKey.c_str(), serverKey.size(), (unsigned char *)shaHash);
	
	SHA1 sha1;
	sha1.SHA_GO(serverKey.c_str(), shaHash);
	//serverKey = Base64::base64_encode(std::string(shaHash)) + "\r\n\r\n";
	serverKey = Base64::base64_encode((unsigned char*)shaHash, strlen(shaHash)) + "\r\n\r\n";
	std::string strtmp(serverKey.c_str());
	response += strtmp;

	return ret;
}


//����
int WebSokcet::Decode(std::string inFrame, std::string &outMessage)
{
	int ret = WS_OPENING_FRAME;
	const char *frameData = inFrame.c_str();
	const int frameLength = inFrame.size();
	if (frameLength < 2)
	{
		ret = WS_ERROR_FRAME;
	}

	// �����չλ������
	if ((frameData[0] & 0x70) != 0x0)
	{
		ret = WS_ERROR_FRAME;
	}

	// finλ: Ϊ1��ʾ�ѽ�����������, Ϊ0��ʾ����������������
	ret = (frameData[0] & 0x80);
	if ((frameData[0] & 0x80) != 0x80)
	{
		ret = WS_ERROR_FRAME;
	}

	// maskλ, Ϊ1��ʾ���ݱ�����
	if ((frameData[1] & 0x80) != 0x80)
	{
		ret = WS_ERROR_FRAME;
	}

	// ������
	uint16_t payloadLength = 0;
	uint8_t payloadFieldExtraBytes = 0;
	uint8_t opcode = static_cast<uint8_t>(frameData[0] & 0x0f);
	if (opcode == WS_TEXT_FRAME)
	{
		// ����utf-8������ı�֡
		payloadLength = static_cast<uint16_t>(frameData[1] & 0x7f);
		if (payloadLength == 0x7e)
		{
			uint16_t payloadLength16b = 0;
			payloadFieldExtraBytes = 2;
			memcpy(&payloadLength16b, &frameData[2], payloadFieldExtraBytes);
			payloadLength = ntohs(payloadLength16b);
		}
		else if (payloadLength == 0x7f)
		{
			// ���ݹ���,�ݲ�֧��
			ret = WS_ERROR_FRAME;
		}
	}
	else if (opcode == WS_BINARY_FRAME || opcode == WS_PING_FRAME || opcode == WS_PONG_FRAME)
	{
		// ������/ping/pong֡�ݲ�����
	}
	else if (opcode == WS_CLOSING_FRAME)
	{
		ret = WS_CLOSING_FRAME;
	}
	else
	{
		ret = WS_ERROR_FRAME;
	}

	// ���ݽ���
	if ((ret != WS_ERROR_FRAME) && (payloadLength > 0))
	{
		// header: 2�ֽ�, masking key: 4�ֽ�
		const char *maskingKey = &frameData[2 + payloadFieldExtraBytes];
		char *payloadData = new char[payloadLength + 1];
		memset(payloadData, 0, payloadLength + 1);
		memcpy(payloadData, &frameData[2 + payloadFieldExtraBytes + 4], payloadLength);
		for (int i = 0; i < payloadLength; i++)
		{
			payloadData[i] = payloadData[i] ^ maskingKey[i % 4];
		}

		outMessage = payloadData;
		delete[] payloadData;
	}

	return ret;
}


//����
int WebSokcet::Encode(std::string inMessage, std::string &outFrame, enum WS_FrameType frameType)
{
	int ret = WS_EMPTY_FRAME;
	const uint32_t messageLength = inMessage.size();
	if (messageLength > 32767)
	{
		// �ݲ�֧����ô��������
		return WS_ERROR_FRAME;
	}

	uint8_t payloadFieldExtraBytes = (messageLength <= 0x7d) ? 0 : 2;
	// header: 2�ֽ�, maskλ����Ϊ0(������), ������masking key������д, ʡ��4�ֽ�
	uint8_t frameHeaderSize = 2 + payloadFieldExtraBytes;
	uint8_t *frameHeader = new uint8_t[frameHeaderSize];
	memset(frameHeader, 0, frameHeaderSize);
	// finλΪ1, ��չλΪ0, ����λΪframeType
	frameHeader[0] = static_cast<uint8_t>(0x80 | frameType);

	// ������ݳ���
	if (messageLength <= 0x7d)
	{
		frameHeader[1] = static_cast<uint8_t>(messageLength);
	}
	else
	{
		frameHeader[1] = 0x7e;
		uint16_t len = htons(messageLength);
		memcpy(&frameHeader[2], &len, payloadFieldExtraBytes);
	}

	// �������
	uint32_t frameSize = frameHeaderSize + messageLength;
	char *frame = new char[frameSize + 1];
	memcpy(frame, frameHeader, frameHeaderSize);
	memcpy(frame + frameHeaderSize, inMessage.c_str(), messageLength);
	frame[frameSize] = '\0';
	outFrame = frame;

	delete[] frame;
	delete[] frameHeader;
	return ret;

}