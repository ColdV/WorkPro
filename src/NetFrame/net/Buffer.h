#pragma once 

#include "../../common/stdafx.h"
#include "Socket.h"
#include <list>



namespace NetFrame
{
	const uint32 BUFFER_INIT_SIZE = 1024;

	class Buffer
	{
	public:
		struct BufferNode
		{
			char* buffer;
			char* readCursor;		//��ǰ������
			uint32 totalSize;		
			uint32 useSize;			//��д���С
		};

		typedef std::list<BufferNode*>	BufferList;
		typedef std::list<BufferNode*>::iterator BufferNodeIt;

	public:
		Buffer();
		virtual ~Buffer();

		//��socket�ж�������   д�뵽m_bufferList
		int ReadSocket(Socket* pSocket);

		//����m_bufferList�е�����  д�뵽socket
		//���û��ָ��size ����������д��socket
		int WriteSocket(Socket* pSocket, uint32 size = 0);

		//����m_bufferList�е����� д�뵽����buffer��
		uint32 ReadBuffer(char* buffer, uint32 size);

		//��buffer����д�뵽m_bufferList��
		uint32 WriteBuffer(const char* buffer, uint32 size);

		//��д������
		//@size:������д��������С
		char* GetWriteBuffer(uint32* size);

		//�ƶ���д������
		//����д����������ӵ�ǰλ���ƶ�size��λ��
		void MoveWriteBuffer(uint32 size);

	private:
		int Expand();

		//ʹlist��ʹ��ʱ��Ϊ��״
		BufferNodeIt GetNextWNodeIt() { if (++m_wNodeIt == m_buffList.end()) m_wNodeIt = m_buffList.begin(); return m_wNodeIt; }

		BufferNodeIt GetNextRNodeIt() { if (++m_rNodeIt == m_buffList.end()) m_rNodeIt = m_buffList.begin(); return m_rNodeIt; }

		uint32 GetLeftSize() { return m_buffList.size() * BUFFER_INIT_SIZE - m_useSize; }

	private:
		BufferList m_buffList;
		BufferNodeIt m_wNodeIt;				//��ǰд���
		BufferNodeIt m_rNodeIt;				//��ǰ������
		uint32 m_useSize;
	};
}