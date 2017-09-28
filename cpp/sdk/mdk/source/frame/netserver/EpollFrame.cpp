// EpollFrame.cpp: implementation of the EpollFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "../../../include/frame/netserver/EpollMonitor.h"
#include "../../../include/frame/netserver/EpollFrame.h"
#include "../../../include/frame/netserver/NetConnect.h"
#include "../../../include/mdk/atom.h"
#include "../../../include/mdk/Lock.h"
#include "../../../include/mdk/Socket.h"
using namespace std;

#ifndef WIN32
#include <sys/epoll.h>
#include <cstdlib>
#include <cstdio>
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace mdk
{

EpollFrame::EpollFrame()
{
#ifndef WIN32
	m_pNetMonitor = new EpollMonitor;
#endif
}

EpollFrame::~EpollFrame()
{
#ifndef WIN32
	if ( NULL != m_pNetMonitor ) 
	{
		delete m_pNetMonitor;
		m_pNetMonitor = NULL;
	}
#endif
}

void* EpollFrame::NetMonitor( void* pParam )
{
#ifndef WIN32
	int handerType = (uint64)pParam;
	if ( 0 == handerType ) NewConnectMonitor();
	else if ( 1 == handerType ) DataMonitor();
	else if ( 2 == handerType ) SendAbleMonitor();
	return NULL;
#endif
				
	return NULL;
}

void EpollFrame::NewConnectMonitor()
{
#ifndef WIN32
	int nCount = MAXPOLLSIZE;
	epoll_event *events = new epoll_event[nCount];	//epoll�¼�
	int i = 0;
	Socket listenSock;
	Socket clientSock;

	while ( !m_stop )
	{
		nCount = MAXPOLLSIZE;
		if ( !((EpollMonitor*)m_pNetMonitor)->WaitConnect( events, nCount, -1 ) ) break; 

		for ( i = 0; i < nCount; i++ )
		{
			if ( ((EpollMonitor*)m_pNetMonitor)->IsStop(events[i].data.u64) ) 
			{
				delete[]events;
				return;
			}

			listenSock.Detach();
			listenSock.Attach((int)events[i].data.u64);
			while ( true )
			{
				listenSock.Accept( clientSock );
				if ( INVALID_SOCKET == clientSock.GetSocket() ) 
				{
					clientSock.Detach();
					break;
				}
				OnConnect(clientSock.Detach(), 0);
			}
			if ( !m_pNetMonitor->AddAccept( listenSock.GetSocket() ) ) 
			{
				printf( "AddAccept (%d) error \n", listenSock.GetSocket() );
				listenSock.Close();
			}
		}
	}
	delete[]events;
#endif
}

void EpollFrame::DataMonitor()
{
#ifndef WIN32
	int nCount = MAXPOLLSIZE;
	epoll_event *events = new epoll_event[nCount];	//epoll�¼�
	int i = 0;
	map<int64,int> ioList;
	map<int64,int>::iterator it;
	bool ret = false;
	while ( !m_stop )
	{
		//û�п�io��socket��ȴ��¿�io��socket
		//�������Ƿ����µĿ�io��socket������ȡ�����뵽ioList�У�û��Ҳ���ȴ�
		//��������ioList�е�socket����io����
		nCount = MAXPOLLSIZE;
		if ( 0 >= ioList.size() ) ret = ((EpollMonitor*)m_pNetMonitor)->WaitData( events, nCount, -1 );
		else ret = ((EpollMonitor*)m_pNetMonitor)->WaitData( events, nCount, 0 );
		if ( !ret ) break;

		//���뵽ioList��
		for ( i = 0; i < nCount; i++ )
		{
			if ( ((EpollMonitor*)m_pNetMonitor)->IsStop(events[i].data.u64) ) 
			{
				delete[]events;
				return;
			}

			//����recv send����뵽io�б�ͳһ����
			it = ioList.find(events[i].data.u64);
			if ( it != ioList.end() ) continue;
			ioList.insert(map<int64,int>::value_type(events[i].data.u64, 1) );//���ӿ�io�Ķ���
		}
		
		//����ioList��ִ��1��io
		for ( it = ioList.begin(); it != ioList.end(); it++ )
		{
			if ( 1&it->second ) //�ɶ�
			{
				if ( ok != OnData( it->first, 0, 0 ) ) //�����Ѷ���������ѶϿ�
				{
					it->second = it->second&~1;//����¼�
				}
			}
		}
	
		//������io��socket���
		it = ioList.begin();
		while (  it != ioList.end() ) 
		{
			if ( 0 == it->second ) 
			{
				ioList.erase(it);
				it = ioList.begin();
			}
			else it++;
		}
	}

#endif
}

void EpollFrame::SendAbleMonitor()
{
#ifndef WIN32
	int nCount = MAXPOLLSIZE;
	epoll_event *events = new epoll_event[nCount];	//epoll�¼�
	int i = 0;
	map<int64,int> ioList;
	map<int64,int>::iterator it;
	bool ret = false;
	while ( !m_stop )
	{
		//û�п�io��socket��ȴ��¿�io��socket
		//�������Ƿ����µĿ�io��socket������ȡ�����뵽ioList�У�û��Ҳ���ȴ�
		//��������ioList�е�socket����io����
		nCount = MAXPOLLSIZE;
		if ( 0 >= ioList.size() ) ret = ((EpollMonitor*)m_pNetMonitor)->WaitSendable( events, nCount, -1 );
		else ret = ((EpollMonitor*)m_pNetMonitor)->WaitSendable( events, nCount, 0 );
		if ( !ret ) break;

		//���뵽ioList��
		for ( i = 0; i < nCount; i++ )
		{
			if ( ((EpollMonitor*)m_pNetMonitor)->IsStop(events[i].data.u64) ) 
			{
				delete[]events;
				return;
			}

			//����recv send����뵽io�б�ͳһ����
			it = ioList.find(events[i].data.u64);
			if ( it != ioList.end() ) continue;
			ioList.insert(map<int64,int>::value_type(events[i].data.u64, 2) );//���ӿ�io�Ķ���
		}

		//����ioList��ִ��1��io
		for ( it = ioList.begin(); it != ioList.end(); it++ )
		{
			if ( 2&it->second ) //��д
			{
				if ( ok != OnSend( it->first, 0 ) )//�����Ѿ������꣬��socket�Ѿ��Ͽ�����socket����д
				{
					it->second = it->second&~2;//����¼�
				}
			}
		}

		//������io��socket���
		it = ioList.begin();
		while (  it != ioList.end() ) 
		{
			if ( 0 == it->second ) 
			{
				ioList.erase(it);
				it = ioList.begin();
			}
			else it++;
		}
	}
#endif
}

connectState EpollFrame::RecvData( NetConnect *pConnect, char *pData, unsigned short uSize )
{
#ifndef WIN32
	unsigned char* pWriteBuf = NULL;	
	int nRecvLen = 0;
	unsigned int nMaxRecvSize = 0;
	//������1M���ݣ��ø��������ӽ���io
	while ( nMaxRecvSize < 1048576 )
	{
		pWriteBuf = pConnect->PrepareBuffer(BUFBLOCK_SIZE);
		nRecvLen = pConnect->GetSocket()->Receive(pWriteBuf, BUFBLOCK_SIZE);
		if ( nRecvLen < 0 ) return unconnect;
		if ( 0 == nRecvLen ) 
		{
			int64 connectId = pConnect->GetID();
			if ( !m_pNetMonitor->AddRecv(pConnect->GetSocket()->GetSocket(), (char*)&connectId, sizeof(int64) ) ) return unconnect;
			return wait_recv;
		}
		nMaxRecvSize += nRecvLen;
		pConnect->WriteFinished( nRecvLen );
	}
#endif
	return ok;
}

int EpollFrame::ListenPort(int port)
{
#ifndef WIN32
	Socket listenSock;//����socket
	if ( !listenSock.Init( Socket::tcp ) ) 
	{
		printf( "listen %d error not create socket\n", port );
		return INVALID_SOCKET;
	}
	listenSock.SetSockMode();
	if ( !listenSock.StartServer( port ) ) 
	{
		printf( "listen %d error port is using\n", port );
		listenSock.Close();
		return INVALID_SOCKET;
	}
	if ( !((EpollMonitor*)m_pNetMonitor)->AddConnectMonitor( listenSock.GetSocket() ) )
	{
		printf( "listen %d error AddConnectMonitor\n", port );
		listenSock.Close();
		return INVALID_SOCKET;
	}
	if ( !m_pNetMonitor->AddAccept( listenSock.GetSocket() ) ) 
	{
		printf( "listen %d error AddAccept\n", port );
		listenSock.Close();
		return INVALID_SOCKET;
	}

	return listenSock.Detach();
#endif
	return INVALID_SOCKET;
}

connectState EpollFrame::SendData(NetConnect *pConnect, unsigned short uSize)
{
#ifndef WIN32
	connectState cs = wait_send;//Ĭ��Ϊ�ȴ�״̬
	//////////////////////////////////////////////////////////////////////////
	//ִ�з���
	unsigned char buf[BUFBLOCK_SIZE];
	int nSize = 0;
	int nSendSize = 0;
	int nFinishedSize = 0;
	nSendSize = pConnect->m_sendBuffer.GetLength();
	if ( 0 < nSendSize )
	{
		nSize = 0;
		//һ�η���4096byte
		if ( BUFBLOCK_SIZE < nSendSize )//1�η����꣬����Ϊ����״̬
		{
			pConnect->m_sendBuffer.ReadData(buf, BUFBLOCK_SIZE, false);
			nSize += BUFBLOCK_SIZE;
			nSendSize -= BUFBLOCK_SIZE;
			cs = ok;
		}
		else//1�οɷ��꣬����Ϊ�ȴ�״̬
		{
			pConnect->m_sendBuffer.ReadData(buf, nSendSize, false);
			nSize += nSendSize;
			nSendSize = 0;
			cs = wait_send;
		}
		nFinishedSize = pConnect->GetSocket()->Send((char*)buf, nSize);//����
		if ( Socket::seError == nFinishedSize ) cs = unconnect;
		else
		{
			pConnect->m_sendBuffer.ReadData(buf, nFinishedSize);//�����ͳɹ������ݴӻ������
			if ( nFinishedSize < nSize ) //sock��д��������Ϊ�ȴ�״̬
			{
				cs = wait_send;
			}
		}
	}
	if ( ok == cs || unconnect == cs ) return cs;//����״̬�����ӹر�ֱ�ӷ��أ����ӹرղ��ؽ����������̣�pNetConnect����ᱻ�ͷţ����������Զ�����

	//�ȴ�״̬���������η��ͣ��������·�������
	pConnect->SendEnd();//���ͽ���
	//////////////////////////////////////////////////////////////////////////
	//����Ƿ���Ҫ��ʼ�µķ�������
	if ( 0 >= pConnect->m_sendBuffer.GetLength() ) return cs;
	/*
		�ⲿ�����߳�����ɷ��ͻ���д��
		���̲߳���SendStart()��ֻ��һ���ɹ�
		���ۣ�������ֲ������ͣ�Ҳ����©����
	*/
	if ( !pConnect->SendStart() ) return cs;//�Ѿ��ڷ���
	//�������̿�ʼ
	int64 connectId = pConnect->GetID();
	if ( !m_pNetMonitor->AddSend( pConnect->GetSocket()->GetSocket(), (char*)&connectId, sizeof(int64) ) ) cs = unconnect;

	return cs;
#endif
	return ok;
}

}//namespace mdk
