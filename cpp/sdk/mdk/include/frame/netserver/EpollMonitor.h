// EpollMonitor.h: interface for the EpollMonitor class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MDK_EPOLLMONITOR_H
#define MDK_EPOLLMONITOR_H

#include "../../../include/mdk/Thread.h"
#include "../../../include/mdk/Task.h"
#include "../../../include/mdk/Queue.h"
#include "../../../include/mdk/Signal.h"
#include "../../../include/mdk/Lock.h"
#include "NetEventMonitor.h"
#include <map>
#include <vector>

namespace mdk
{

class EpollMonitor : public NetEventMonitor  
{
public:
	enum EventType
	{
		epoll_accept = 0,
		epoll_in = 1,
		epoll_out = 2,
	};
	typedef struct IO_EVENT
	{
		int sock;
		EventType type;
	}IO_EVENT;
public:
	EpollMonitor();
	virtual ~EpollMonitor();

public:
	//��ʼ����
	bool Start( int nMaxMonitor );
	//ֹͣ����
	bool Stop();
	//����һ��Accept�������������Ӳ���
	bool AddAccept( int sock );
	//����һ���������ݵĲ����������ݵ���
	bool AddRecv( int sock, char* pData, unsigned short dataSize );
	//����һ���������ݵĲ������������
	bool AddSend( int sock, char* pData, unsigned short dataSize );
	//�ȴ��¼�����
	//ɾ��һ����������Ӽ����б�
	bool DelMonitor( int sock );
	
	bool AddConnectMonitor( int sock );
	bool AddDataMonitor( int sock, char* pData, unsigned short dataSize );
	bool AddSendableMonitor( int sock, char* pData, unsigned short dataSize );
	bool WaitConnect( void *eventArray, int &count, int timeout );
	bool WaitData( void *eventArray, int &count, int timeout );
	bool WaitSendable( void *eventArray, int &count, int timeout );
	bool IsStop( int64 connectId );

protected:
	void SheildSigPipe();//����SIGPIPE�źţ�������̱����źŹر�
	bool DelMonitorIn( int sock );
	bool DelMonitorOut( int sock );
	
private:
	bool m_bStop;
	int m_nMaxMonitor;//������socket����
	int m_epollExit;//epoll�˳�����sock
	int m_hEPollAccept;//�������¼�������epoll���
	int m_hEPollIn;//EPOLLIN������ epoll���
	int m_hEPollOut;//EPOLLOUT������ epoll���
};

}//namespace mdk

#endif // MDK_EPOLLMONITOR_H
