// IOCPMonitor.cpp: implementation of the IOCPMonitor class.
//
//////////////////////////////////////////////////////////////////////

#include "../../../include/frame/netserver/IOCPMonitor.h"
#ifdef WIN32
#pragma comment ( lib, "mswsock.lib" )
#pragma comment ( lib, "ws2_32.lib" )
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace mdk
{

IOCPMonitor::IOCPMonitor()
:m_iocpDataPool( sizeof(IOCP_OVERLAPPED), 200 )
{
	m_nCPUCount = 0;
#ifdef WIN32
	SYSTEM_INFO sysInfo;
	::GetSystemInfo(&sysInfo);
	m_nCPUCount = sysInfo.dwNumberOfProcessors;
#endif
}

IOCPMonitor::~IOCPMonitor()
{

}

int IOCPMonitor::GetError(int sock, WSAOVERLAPPED* pWSAOVERLAPPED)  
{  
#ifdef WIN32
	DWORD dwTrans;  
	DWORD dwFlags;  
	if(FALSE == WSAGetOverlappedResult(sock, pWSAOVERLAPPED, &dwTrans, FALSE, &dwFlags))  
		return WSAGetLastError();  
	else  
		return ERROR_SUCCESS;  
#endif
	return 0;
}  

bool IOCPMonitor::Start( int nMaxMonitor )
{
#ifdef WIN32
	//����IOCP����
	//������ɶ˿�
	int NumberOfConcurrentThreads = 0;
	if ( 0 < m_nCPUCount ) NumberOfConcurrentThreads = m_nCPUCount * 2 + 2;
	m_hCompletPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE, 
		0, 0, NumberOfConcurrentThreads );
	if ( NULL == m_hCompletPort ) 
	{
		m_initError = "create iocp monitor faild";
		return false;
	}
	return true;

#endif
	return true;
}

bool IOCPMonitor::AddMonitor( int sock, char* pData, unsigned short dataSize )
{
#ifdef WIN32
	//�������׽��ּ���IOCP�ж�
	if ( NULL == CreateIoCompletionPort( (HANDLE)sock, m_hCompletPort, 
		(DWORD)sock, 0 ) ) return false;
	return true;

#endif
	return true;
}

bool IOCPMonitor::WaitEvent( void *eventArray, int &count, bool block )
{
#ifdef WIN32
	IO_EVENT *events = (IO_EVENT*)eventArray;
	count = 0;
	/*
		��ɲ����ϵĴ��䳤��
		����accept�������ʱΪ0
		����recv/send�������ʱ����¼����/�����ֽ���
		���recv/send�����ϵĴ��䳤��С�ڵ���0��ʾ�׽����ѹر�
	*/
	DWORD dwIOSize;
	IOCP_OVERLAPPED *pOverlapped = NULL;//�����������
	int sock;
	if ( !::GetQueuedCompletionStatus( m_hCompletPort, &dwIOSize, 
						(LPDWORD)&sock, (OVERLAPPED**)&pOverlapped,
						INFINITE ) )
	{
		DWORD dwErrorCode = GetLastError();
		if ( ERROR_INVALID_HANDLE == dwErrorCode ) //����ʱ��δ֪ԭ��Ƿ�����������ǲ���ǰһ�γ���رպ�δ���������¼�
		{
			try
			{
				if ( NULL != pOverlapped )
				{
					m_iocpDataPool.Free(pOverlapped);
					pOverlapped = NULL;
					return true;
				}
			}catch(...)
			{
			}
			return true;
		}
		
		if ( ERROR_OPERATION_ABORTED == dwErrorCode ) 
		{
			if ( IOCPMonitor::connect == pOverlapped->completiontype ) 
			{
				m_iocpDataPool.Free(pOverlapped);
				pOverlapped = NULL;
				return false;
			}
		}
		if ( IOCPMonitor::connect == pOverlapped->completiontype ) //Accept�ϵ�socket�رգ�����Ͷ�ݼ���
		{
			AddAccept(sock);
		}
		else//�ͻ����쳣�Ͽ��������ߣ��ϵ磬��ֹ����
		{
			events[count].connectId = pOverlapped->connectId;
			events[count].type = IOCPMonitor::close;
			count++;
		}
	}
	else if ( IOCPMonitor::close == pOverlapped->completiontype )
	{
		Stop();
		return false;
	}
	else if ( 0 == dwIOSize && IOCPMonitor::recv == pOverlapped->completiontype )
	{
		events[count].connectId = pOverlapped->connectId;
		events[count].type = IOCPMonitor::close;
		count++;
	}
	else//io�¼�
	{
		if ( IOCPMonitor::connect == pOverlapped->completiontype )
		{
			AddAccept( sock );
			events[count].listenSock = sock;
		}
		//io�¼�
		events[count].connectId = pOverlapped->connectId;
		events[count].type = pOverlapped->completiontype;
		events[count].client = pOverlapped->sock;
		events[count].pData = pOverlapped->m_wsaBuffer.buf;
		events[count].uDataSize = (unsigned short)dwIOSize;
		count++;
	}
	m_iocpDataPool.Free(pOverlapped);
	pOverlapped = NULL;
				
	return true;
	
#endif
	return true;
}

bool IOCPMonitor::AddAccept( int listenSocket )
{
#ifdef WIN32
	if ( SOCKET_ERROR == listenSocket ) return false;
	//��������
	IOCP_OVERLAPPED *pOverlapped = new (m_iocpDataPool.Alloc())IOCP_OVERLAPPED;
	if ( NULL == pOverlapped ) return false;
	memset( &pOverlapped->m_overlapped, 0, sizeof(OVERLAPPED) );
	pOverlapped->m_dwLocalAddressLength = sizeof(SOCKADDR_IN) + 16;//�ͻ��˾�����IP
	pOverlapped->m_dwRemoteAddressLength = sizeof(SOCKADDR_IN) + 16;//�ͻ�������IP
	memset( pOverlapped->m_outPutBuf, 0, sizeof(SOCKADDR_IN)*2+32 );
	Socket client;
	client.Init( Socket::tcp );
	pOverlapped->sock = client.GetSocket();
	pOverlapped->completiontype = IOCPMonitor::connect;
	//Ͷ�ݽ������Ӳ���
	if ( !AcceptEx( listenSocket,
		client.GetSocket(), 
		pOverlapped->m_outPutBuf, 0,
		pOverlapped->m_dwLocalAddressLength, 
		pOverlapped->m_dwRemoteAddressLength, 
		NULL, &pOverlapped->m_overlapped ) )
	{
		int nErrCode = WSAGetLastError();
		if ( ERROR_IO_PENDING != nErrCode ) 
		{
			m_iocpDataPool.Free(pOverlapped);
			pOverlapped = NULL;
			return false;
		}
	}
	
	return true;
	
#endif
	return true;
}

//����һ���������ݵĲ����������ݵ��WaitEvent�᷵��
bool IOCPMonitor::AddRecv( int socket, char* pData, unsigned short dataSize )
{
#ifdef WIN32
	IOCP_DATA *pIocpData = (IOCP_DATA *)pData;
	IOCP_OVERLAPPED *pOverlapped = new (m_iocpDataPool.Alloc())IOCP_OVERLAPPED;
	if ( NULL == pOverlapped )return false;
	memset( &pOverlapped->m_overlapped, 0, sizeof(OVERLAPPED) );
	pOverlapped->m_wsaBuffer.buf = pIocpData->buf;
	pOverlapped->m_wsaBuffer.len = pIocpData->bufSize;
	pOverlapped->m_overlapped.Internal = 0;
	pOverlapped->connectId = pIocpData->connectId;
	pOverlapped->sock = socket;
	pOverlapped->completiontype = IOCPMonitor::recv;
	
	DWORD dwRecv = 0;
	DWORD dwFlags = 0;
	//Ͷ�����ݽ��ղ���
	if ( ::WSARecv( socket, 
		&pOverlapped->m_wsaBuffer, 
		1, &dwRecv, &dwFlags, 
		&pOverlapped->m_overlapped, NULL ) )
	{
		int nErrCode = WSAGetLastError();
		if ( ERROR_IO_PENDING != nErrCode ) 
		{
			m_iocpDataPool.Free(pOverlapped);
			pOverlapped = NULL;
			return false;
		}
	}
	return true;
	
#endif
	return true;
}

//����һ���������ݵĲ�����������ɣ�WaitEvent�᷵��
bool IOCPMonitor::AddSend( int socket, char* pData, unsigned short dataSize )
{
#ifdef WIN32
	IOCP_DATA *pIocpData = (IOCP_DATA *)pData;
	IOCP_OVERLAPPED *pOverlapped = new (m_iocpDataPool.Alloc())IOCP_OVERLAPPED;
	if ( NULL == pOverlapped ) return false;
	memset( &pOverlapped->m_overlapped, 0, sizeof(OVERLAPPED) );
	pOverlapped->m_wsaBuffer.buf = pIocpData->buf;
	pOverlapped->m_wsaBuffer.len = pIocpData->bufSize;
	pOverlapped->m_overlapped.Internal = 0;
	pOverlapped->connectId = pIocpData->connectId;
	pOverlapped->sock = socket;
	pOverlapped->completiontype = IOCPMonitor::send;
	
	DWORD dwSend = 0;
	DWORD dwFlags = 0;
	//Ͷ�����ݽ��ղ���
	if ( ::WSASend( socket, 
		&pOverlapped->m_wsaBuffer, 
		1, &dwSend, dwFlags, 
		&pOverlapped->m_overlapped, NULL ) )
	{
		int nErrCode = WSAGetLastError();
		if ( ERROR_IO_PENDING != nErrCode ) 
		{
			m_iocpDataPool.Free(pOverlapped);
			pOverlapped = NULL;
			return false;
		}
	}
	return true;
	
#endif
	return true;
}

bool IOCPMonitor::Stop()
{
#ifdef WIN32
	memset( &m_stopOverlapped.m_overlapped, 0, sizeof(OVERLAPPED) );
	m_stopOverlapped.m_wsaBuffer.buf = NULL;
	m_stopOverlapped.m_wsaBuffer.len = 0;
	m_stopOverlapped.m_overlapped.Internal = 0;
	m_stopOverlapped.sock = 0;
	m_stopOverlapped.completiontype = IOCPMonitor::close;
	
	::PostQueuedCompletionStatus(m_hCompletPort, 0, 0, (OVERLAPPED*)&m_stopOverlapped );
#endif
	return true;

}

}//namespace mdk
