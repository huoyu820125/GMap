// NetConnect.cpp: implementation of the NetConnect class.
//
//////////////////////////////////////////////////////////////////////

#include "../../../include/frame/netserver/NetConnect.h"
#include "../../../include/frame/netserver/NetEventMonitor.h"
#include "../../../include/frame/netserver/NetEngine.h"
#include "../../../include/frame/netserver/HostData.h"
#include "../../../include/mdk/atom.h"
#include "../../../include/mdk/MemoryPool.h"
#include "../../../include/mdk/mapi.h"
using namespace std;

namespace mdk
{

NetConnect::NetConnect(int sock, int listenSock, bool bIsServer, NetEventMonitor *pNetMonitor, NetEngine *pEngine, MemoryPool *pMemoryPool)
:m_socket(sock,Socket::tcp)
{
	m_pMemoryPool = pMemoryPool;
	m_useCount = 1;
	m_pEngine = pEngine;
	m_pNetMonitor = pNetMonitor;
	m_id = -1;
	m_host.m_pConnect = this;
	m_nReadCount = 0;
	m_bReadAble = false;

	m_nSendCount = 0;//���ڽ��з��͵��߳���
	m_bSendAble = false;//io��������������Ҫ����
	m_bConnect = true;//ֻ�з������ӲŴ����������Զ��󴴽�����һ��������״̬
	m_nDoCloseWorkCount = 0;//û��ִ�й�NetServer::OnClose()
	m_bIsServer = bIsServer;
#ifdef WIN32
	Socket::InitForIOCP(sock, listenSock);
#endif
	m_socket.InitPeerAddress();
	m_socket.InitLocalAddress();
 	m_pHostData = NULL;
	m_autoFreeData = true;
	m_pSvrInfo = NULL;
}

NetConnect::~NetConnect()
{
	/*
		���ü��m_autoFree
		if m_autoFree = true,Ӧ���ͷ�
		if m_autoFree = false
			if data��host����˹���,��m_pHostData=NULL
			else if data������m_pHostData=NULL
			else data�д���host������,host�����ܱ�����
		����NULL != m_pHostData��һ���Ǵ���ģʽ��ִ��Release()
	*/
	if ( NULL != m_pHostData ) m_pHostData->Release();
	m_pHostData = NULL;
}

void NetConnect::Release()
{
	if ( 1 == AtomDec(&m_useCount, 1) )
	{
		m_host.m_pConnect = NULL;
		if ( NULL == m_pMemoryPool ) 
		{
			delete this;
			return;
		}
		this->~NetConnect();
		m_pMemoryPool->Free(this);
	}
}

void NetConnect::SetID( int64 connectId )
{
	m_id = connectId;
}

void NetConnect::RefreshHeart()
{
	m_tLastHeart = time( NULL );
}

time_t NetConnect::GetLastHeart()
{
	return m_tLastHeart;
}

unsigned char* NetConnect::PrepareBuffer(unsigned short uRecvSize)
{
	return m_recvBuffer.PrepareBuffer( uRecvSize );
}

void NetConnect::WriteFinished(unsigned short uLength)
{
	m_recvBuffer.WriteFinished( uLength );
}

bool NetConnect::IsReadAble()
{
	return m_bReadAble && 0 < m_recvBuffer.GetLength();
}

uint32 NetConnect::GetLength()
{
	return m_recvBuffer.GetLength();
}

bool NetConnect::ReadData( unsigned char* pMsg, unsigned int uLength, bool bClearCache )
{
	m_bReadAble = m_recvBuffer.ReadData( pMsg, uLength, bClearCache );
	if ( !m_bReadAble ) uLength = 0;
	
	return m_bReadAble;
}

bool NetConnect::SendData( const unsigned char* pMsg, unsigned int uLength )
{
	unsigned char *ioBuf = NULL;
	int nSendSize = 0;
	AutoLock lock(&m_sendMutex);//�ظ�������֪ͨ���ڲ���send
	if ( 0 >= m_sendBuffer.GetLength() )//û�еȴ����͵����ݣ���ֱ�ӷ���
	{
		nSendSize = m_socket.Send( pMsg, uLength );
	}
	if ( Socket::seError == nSendSize ) return false;//�����������ӿ����ѶϿ�
	if ( uLength == nSendSize ) return true;//���������ѷ��ͣ����سɹ�
	
	//���ݼ��뷢�ͻ��壬�����ײ�ȥ����
	uLength -= nSendSize;
	while ( true )
	{
		if ( uLength > BUFBLOCK_SIZE )
		{
			ioBuf = m_sendBuffer.PrepareBuffer( BUFBLOCK_SIZE );
			memcpy( ioBuf, &pMsg[nSendSize], BUFBLOCK_SIZE );
			m_sendBuffer.WriteFinished( BUFBLOCK_SIZE );
			nSendSize += BUFBLOCK_SIZE;
			uLength -= BUFBLOCK_SIZE;
		}
		else
		{
			ioBuf = m_sendBuffer.PrepareBuffer( uLength );
			memcpy( ioBuf, &pMsg[nSendSize], uLength );
			m_sendBuffer.WriteFinished( uLength );
			break;
		}
	}
	if ( !SendStart() ) return true;//�Ѿ��ڷ���
#ifdef WIN32
	IOCP_DATA iocpData;
	iocpData.connectId = m_id;
	iocpData.buf = NULL; 
	iocpData.bufSize = 0;
	return m_pNetMonitor->AddSend( m_socket.GetSocket(), (char*)&iocpData, sizeof(IOCP_DATA) );
#else
	return m_pNetMonitor->AddSend( m_socket.GetSocket(), (char*)&m_id, sizeof(int) );
#endif
	return true;
}

Socket* NetConnect::GetSocket()
{
	return &m_socket;
}

int64 NetConnect::GetID()
{
	return m_id;
}

//��ʼ��������
bool NetConnect::SendStart()
{
	if ( 0 != AtomAdd(&m_nSendCount,1) ) return false;//ֻ�������һ����������
	return true;
}

//������������
void NetConnect::SendEnd()
{
	m_nSendCount = 0;
}

void NetConnect::Close()
{
	m_pEngine->CloseConnect(m_id);
}

bool NetConnect::IsServer()
{
	return m_bIsServer;
}

void NetConnect::InGroup( int groupID )
{
	m_groups.insert(map<int,int>::value_type(groupID,groupID));
}

void NetConnect::OutGroup( int groupID )
{
	map<int,int>::iterator it;
	it = m_groups.find(groupID);
	if ( it == m_groups.end() ) return;
	m_groups.erase(it);
}

bool NetConnect::IsInGroups( int *groups, int count )
{
	int i = 0;
	for ( i = 0; i < count; i++ )
	{
		if ( m_groups.end() != m_groups.find(groups[i]) ) return true;
	}
	
	return false;
}

void NetConnect::GetServerAddress( string &ip, int &port )
{
	if ( this->m_bIsServer ) m_socket.GetPeerAddress( ip, port );
	else m_socket.GetLocalAddress( ip, port );
	return;
}

void NetConnect::GetAddress( string &ip, int &port )
{
	if ( !this->m_bIsServer ) m_socket.GetPeerAddress( ip, port );
	else m_socket.GetLocalAddress( ip, port );
	return;
}

void NetConnect::SetData( HostData *pData, bool autoFree )
{
	m_autoFreeData = autoFree;
	//�ͷ������õ����ݻ����ü���
	if ( NULL != m_pHostData ) 
	{
		if ( m_pHostData->m_autoFree ) return;//����ģʽ�����ܽ������
		NetHost unconnectHost;
		m_pHostData->SetHost(&unconnectHost);//������δ���ӵ�host
		mdk::AutoLock lock(&m_mutexData);
		m_pHostData->Release();//�ͷ�����,����GetData()�������ο�HostData::Release()�ڲ�ע��
		/*
			��GetData����
			���ܷ���NULL��Ҳ���ܷ�����HostData��Ҳ���ܷ�����Ч��HostData
			������ǰ�����ִ�У����Ҳһ��
			���������SetData��lock���ƣ�Ҳ�����ж�����
		*/
		m_pHostData = NULL;//���host��data�İ�
	}

	if ( NULL == pData ) return;

	if ( -1 == AtomAdd(&pData->m_refCount, 1) ) return; //���߳������Release()���ͷż�飬���󼴽����ͷţ���������
	pData->m_autoFree = autoFree;
	/*
		autoFree = true
		HostData�����������ɿ�ܹ���
		��ܱ�֤�ˣ���ַ����Ч�ԣ�
		ֱ�Ӹ���NetHostָ��,��HostData��,���������ü���.
		

		autoFree = false
		HostData�������������û����й�����ܲ���
		����NetHost����HostData��,���ü������ӣ���ʾ���û��ڷ���NetHost����
		ȷ�����û��ͷ�HostData֮ǰ,NetHost��Զ���ᱻ����ͷš�
	*/
	if ( autoFree ) 
	{
		/*
			����ģʽ������Ҫ�������ü�����������
			ǰ��+1�����ͨ������ֵ��СӦ��=1������˵��+1֮���ⲿ�ַ������ظ�Release()���ǲ�����ģ���������
		*/
		if ( 1 > AtomDec(&pData->m_refCount, 1) ) 
		{
			mdk::mdk_assert(false);
			return; 
		}
		pData->m_pHost = &m_host;//����ģʽ��copy���������ˣ�pData���ᳬ��host�������ڣ�ֻҪpData���ڣ�m_pHost��Ȼ���ڣ�����copy
	}
	else //����ģʽ
	{
		pData->m_hostRef = m_host;//ȷ��m_pHostָ��ȫ�ڴ棬��host��1��copy
	}
	m_pHostData = pData;//�������ȷ��GetData()����ʱ�����ص�ʱ����������

	return;
}

HostData* NetConnect::GetData()
{
	if ( m_autoFreeData ) return m_pHostData;

	mdk::AutoLock lock(&m_mutexData);
	if ( NULL == m_pHostData ) return NULL;
	if ( !m_pHostData->m_autoFree ) 
	{
		//����ģʽ����Ҫ��¼���ü���
		if ( -1 == AtomAdd(&m_pHostData->m_refCount, 1) ) //ȡ����ʱ�����߳�ִ����SetData(NULL)������������Բ�����Release()�ͷ����� 
		{
			return NULL;
		}
	}
	return m_pHostData;
}

void NetConnect::SetSvrInfo(void *pData)
{
	m_pSvrInfo = pData;
}

void* NetConnect::GetSvrInfo()
{
	return m_pSvrInfo;
}

}//namespace mdk

