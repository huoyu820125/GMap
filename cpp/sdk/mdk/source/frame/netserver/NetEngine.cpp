
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#define strnicmp strncasecmp
#endif

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <time.h>

#include "../../../include/mdk/mapi.h"
#include "../../../include/mdk/Socket.h"

#include "../../../include/frame/netserver/NetEngine.h"
#include "../../../include/frame/netserver/NetConnect.h"
#include "../../../include/frame/netserver/NetEventMonitor.h"
#include "../../../include/frame/netserver/NetServer.h"
#include "../../../include/mdk/atom.h"
#include "../../../include/mdk/MemoryPool.h"

using namespace std;
namespace mdk
{

NetEngine::NetEngine()
{
	Socket::SocketInit();
	m_pConnectPool = NULL;
	m_stop = true;//ֹͣ��־
	m_startError = "";
	m_nHeartTime = 0;//�������(S)��Ĭ�ϲ����
	m_pNetMonitor = NULL;
	m_ioThreadCount = 16;//����io�߳�����
	m_workThreadCount = 16;//�����߳�����
	m_pNetServer = NULL;
	m_averageConnectCount = 5000;
	m_nextConnectId = 0;
	m_noDelay = false;
}

NetEngine::~NetEngine()
{
	Stop();
	if ( NULL != m_pConnectPool )
	{
		delete m_pConnectPool;
		m_pConnectPool = NULL;
	}
	Socket::SocketDestory();
}

//����ƽ��������
void NetEngine::SetAverageConnectCount(int count)
{
	m_averageConnectCount = count;
}

//��������ʱ��
void NetEngine::SetHeartTime( int nSecond )
{
	m_nHeartTime = nSecond;
}

//��������IO�߳�����
void NetEngine::SetIOThreadCount(int nCount)
{
	m_ioThreadCount = nCount;//����io�߳�����
}

//���ù����߳���
void NetEngine::SetWorkThreadCount(int nCount)
{
	m_workThreadCount = nCount;//�����߳�����
}

void NetEngine::SetOnWorkStart( MethodPointer method, void *pObj, void *pParam )
{
	m_workThreads.SetOnStart(method, pObj, pParam);
}

void NetEngine::SetOnWorkStart( FuntionPointer fun, void *pParam )
{
	m_workThreads.SetOnStart(fun, pParam);
}

/**
 * ��ʼ����
 * �ɹ�����true��ʧ�ܷ���false
 */
bool NetEngine::Start()
{
	if ( !m_stop ) return true;
	m_stop = false;	
	int memoryCount = 2;
	for ( memoryCount = 2; memoryCount * memoryCount < m_averageConnectCount * 2; memoryCount++ );
	if ( memoryCount < 200 ) memoryCount = 200;
	if ( NULL != m_pConnectPool )//֮ǰStop��������Start
	{
		delete m_pConnectPool;
		m_pConnectPool = NULL;
	}
	m_pConnectPool = new MemoryPool( sizeof(NetConnect), memoryCount );
	if ( NULL == m_pConnectPool )
	{
		m_startError = "�ڴ治�㣬�޷�����NetConnect�ڴ��";
		Stop();
		return false;
	}
	if ( !m_pNetMonitor->Start( MAXPOLLSIZE ) ) 
	{
		m_startError = m_pNetMonitor->GetInitError();
		Stop();
		return false;
	}
	m_workThreads.Start( m_workThreadCount );
	int i = 0;
	for ( i = 0; i < m_ioThreadCount; i++ ) m_ioThreads.Accept( Executor::Bind(&NetEngine::NetMonitorTask), this, NULL);
#ifndef WIN32
	for ( i = 0; i < m_ioThreadCount; i++ ) m_ioThreads.Accept( Executor::Bind(&NetEngine::NetMonitorTask), this, (void*)1 );
	for ( i = 0; i < m_ioThreadCount; i++ ) m_ioThreads.Accept( Executor::Bind(&NetEngine::NetMonitorTask), this, (void*)2 );
	m_ioThreads.Start( m_ioThreadCount * 3 );
#else
	m_ioThreads.Start( m_ioThreadCount );
#endif
	
	if ( !ListenAll() )
	{
		Stop();
		return false;
	}
	ConnectAll();
	m_connectThread.Run( Executor::Bind(&NetEngine::ConnectThread), this, 0 );
	return m_mainThread.Run( Executor::Bind(&NetEngine::Main), this, 0 );
}

void* NetEngine::NetMonitorTask( void* pParam)
{
	return NetMonitor( pParam );
}

//�ȴ�ֹͣ
void NetEngine::WaitStop()
{
	m_mainThread.WaitStop();
}

//ֹͣ����
void NetEngine::Stop()
{
	if ( m_stop ) return;
	m_stop = true;
	m_pNetMonitor->Stop();
	m_sigStop.Notify();
	m_mainThread.Stop( 3000 );
	m_ioThreads.Stop();
	m_workThreads.Stop();
}

//���߳�
void* NetEngine::Main(void*)
{
	while ( !m_stop ) 
	{
		if ( m_sigStop.Wait( 10000 ) ) break;
		HeartMonitor();
		ConnectAll();
	}
	return NULL;
}

//�����߳�
void NetEngine::HeartMonitor()
{
	if ( 0 >= m_nHeartTime ) return;//����������
	//////////////////////////////////////////////////////////////////////////
	//�ر�������������
	ConnectList::iterator it;
	NetConnect *pConnect;
	time_t tCurTime = 0;
	tCurTime = time( NULL );
	time_t tLastHeart;
	AutoLock lock( &m_connectsMutex );
	for ( it = m_connectList.begin();  it != m_connectList.end(); )
	{
		pConnect = it->second;
		if ( pConnect->m_host.IsServer() ) //�������ӣ����������
		{
			it++;
			continue;
		}
		//�������
		tLastHeart = pConnect->GetLastHeart();
		if ( tCurTime < tLastHeart || tCurTime - tLastHeart < m_nHeartTime )//������
		{
			it++;
			continue;
		}
		//������/�����ѶϿ���ǿ�ƶϿ�����
		CloseConnect( it );
		it = m_connectList.begin();
	}
	lock.Unlock();
}

//�ر�һ������
void NetEngine::CloseConnect( ConnectList::iterator it )
{
	/*
	   ������ɾ���ٹرգ�˳���ܻ���
	   ����رպ�eraseǰ��������client���ӽ�����
	   ϵͳ���̾ͰѸ����ӷ������clientʹ�ã������client�ڲ���m_connectListʱʧ��
	*/
	NetConnect *pConnect = it->second;
	m_connectList.erase( it );//֮�󲻿�����MsgWorker()��������ΪOnData�����Ѿ��Ҳ���������
	/*
		pConnect->GetSocket()->Close();
		���ϲ�����V1.51���У����Ӵ˴��ƶ���CloseWorker()��
		��m_pNetServer->OnCloseConnect()֮��ִ��

		A.�����Ƴ�Close��Ŀ��
			��OnCloseConnect()���ǰ��Ҳ����ҵ���������ӶϿ�ҵ��ǰ
			����ϵͳ����socket�ľ����������
			
			�Ա��ⷢ�����������
				�����û���ҵ���(NetServer������)�д���map<int,NetHost>����host�б�
				��NetServer::OnConnect()ʱ����
				��NetServer::OnClose())ʱɾ��

				����������ִ�йر�socket��������Ϊ100��

				ҵ���NetServer::OnClose����֮��õ�֪ͨ��
				�����ʱ���������ӽ�������ϵͳ�����ʹ��100��Ϊ�������������ӡ�
				�����Ƕ��̲߳��������Կ�����NetServer::OnClose֮ǰ����ִ��NetServer::OnConnect()
				����NetServer::OnClose��û����ɣ�100���key���ɴ������û�������map�У�
				����NetServer::OnConnect()�еĲ������ʧ��
				
		  ��ˣ��û���Ҫ׼��һ��wait_insert�жӣ���OnConnect()��insertʧ��ʱ��
		  ��Ҫ�����󱣴浽wait_insert�жӣ�����ֹOnConnect()ҵ���߼�

		  ��OnClose��ɾ��������ö����key��wait_insert�ж��м�飬
		  �ҵ�ƥ��Ķ�����insert��Ȼ�����ִ��OnConnect�ĺ���ҵ��
		  OnConnectҵ���߼��������
		  
		  1.�����Ϸǳ��鷳
		  2.�ƻ��˹����ھۣ�OnConnect()��OnClose()�߼����������һ��

		B.�ٷ����Ƴ�Close��û������������
		����1����������û�йرգ���server������closeʱ������״̬ʵ�ʻ��������ģ�
		���client��ͣ�ط������ݣ��᲻�ᵼ��OnMsg�߳�һֱ���ղ������ݣ�
		��OnCloseû����ִ�У�
		
		  �𣺲��ᣬ��Ϊm_bConnect��־������Ϊfalse�ˣ���OnMsg����MsgWorker()�б�ѭ�����ã�
		ÿ��ѭ��������m_bConnect��־�����Լ�ʹ�������ݿɽ��գ�OnMsgҲ�ᱻ��ֹ
	 */
//	pConnect->GetSocket()->Close();

	pConnect->m_bConnect = false;
	/*
		ִ��ҵ��NetServer::OnClose();
		������δ���MsgWorker������(MsgWorker�ڲ�ѭ������OnMsg())��Ҳ���Ǳ�����OnMsg����

		��MsgWorker�Ĳ����������
		���1��MsgWorker�Ѿ�return
			��ôAtomAdd����0��ִ��NotifyOnClose()�������ܷ�����OnMsg֮ǰ��
			֮��Ҳ������OnMsg��ǰ���Ѿ�˵��MsgWorker()�������ٷ���
		���2��MsgWorkerδ���أ���2�����
			���1��������AtomAdd
				��Ȼ���ط�0����Ϊû�з�����AtomDec
				��ִ��OnClose
				��©OnClose��
				���ᣡ��ô��MsgWorker()��AtomAdd���ط�0������AtomDec��Ȼ����>1��
				MsgWorker()����ѭ��һ��OnMsg�����OnMsg��û�����ݵģ����û�û��Ӱ��
				OnMsg�������㹻���ݺ���������
				Ȼ���˳�ѭ��������m_bConnect=false������NotifyOnClose()����OnClose֪ͨ
				OnClose֪ͨû�б���©
			���2��MsgWorker��AtomDec
				��Ȼ����1����ΪMsgWorkerѭ������������1�����м���û��AtomAdd����
				MsgWorker�˳�ѭ��
				����m_bConnect=false������NotifyOnClose()����OnClose֪ͨ
				Ȼ������AtomAdd��Ȼ����0��ҲNotifyOnClose()����OnClose֪ͨ
				�ظ�֪ͨ��
				���ᣬNotifyOnClose()��֤�˶��̲߳��������£�ֻ��֪ͨ1��

		��OnData�Ĳ����������
			���1��OnData��AtomAdd
				��֤��MsgWorker��ִ��
				AtomAdd���ط�0������NotifyOnClose
				MsgWorkerһ����NotifyOnClose
			���2��������AtomAdd
				OnData��AtomAddʱ��Ȼ����>0��OnData����MsgWorker
				��©OnMsg��Ӧ�������������ݣ���������©
				��3�ֶϿ����
				1.server��������û���ˣ�����close���Ǿ�������ԭ��ǿ�ƶϿ�������ν���ݶ�ʧ
				2.client��server���������ҵ��ϣ�������Ͽ�
					�Ǿ�Ӧ�ð���ͨ����ҵ���Ӱ�ȫ�Ͽ���ԭ���ý��շ�����Close
					�����ܷ��ͷ�����Close,���Բ�������©����
					������ͷ�����close�����������������ƣ���û�취��֤�յ������������
	 */
	NotifyOnClose(pConnect);
	pConnect->Release();//���ӶϿ��ͷŹ������
	return;
}

void NetEngine::NotifyOnClose(NetConnect *pConnect)
{
	if ( 0 == AtomAdd(&pConnect->m_nReadCount, 1) )  
	{
		if ( 0 == AtomAdd(&pConnect->m_nDoCloseWorkCount, 1) )//ֻ��1���߳�ִ��OnClose���ҽ�ִ��1��
		{
			AtomAdd(&pConnect->m_useCount, 1);//ҵ����Ȼ�ȡ����
			m_workThreads.Accept( Executor::Bind(&NetEngine::CloseWorker), this, pConnect);
		}
	}
}

bool NetEngine::OnConnect( int sock, int listenSock, SVR_CONNECT *pSvr )
{
	if ( m_noDelay ) Socket::SetNoDelay(sock, true);
	NetConnect *pConnect = new (m_pConnectPool->Alloc())NetConnect(sock, listenSock, 
		NULL == pSvr?false:true, m_pNetMonitor, this, m_pConnectPool);
	if ( NULL == pConnect ) 
	{
		closesocket(sock);
		return false;
	}
	if ( NULL != pSvr && pSvr->pSvrInfo ) 
	{
		pConnect->SetSvrInfo(pSvr->pSvrInfo);
	}
	pConnect->GetSocket()->SetSockMode();
	//��������б�
	AutoLock lock( &m_connectsMutex );
	int64 connectId = -1;
	while ( -1 == connectId )//����Ԥ��ID
	{
		connectId = m_nextConnectId;
		m_nextConnectId++;
	}
	pConnect->SetID(connectId);
	pConnect->RefreshHeart();
	pair<ConnectList::iterator, bool> ret = m_connectList.insert( ConnectList::value_type(connectId,pConnect) );
	AtomAdd(&pConnect->m_useCount, 1);//ҵ����Ȼ�ȡ����
	lock.Unlock();
	//ִ��ҵ��
	m_workThreads.Accept( Executor::Bind(&NetEngine::ConnectWorker), this, pConnect );
	return true;
}

void* NetEngine::ConnectWorker( NetConnect *pConnect )
{
	int64 connectId = pConnect->GetID();
	bool successed;
#ifdef WIN32
	successed = m_pNetMonitor->AddMonitor(pConnect->GetSocket()->GetSocket(), (char*)&connectId, sizeof(int64) );
#else
	pConnect->SendStart();
	successed = m_pNetMonitor->AddSendableMonitor(pConnect->GetSocket()->GetSocket(), (char*)&connectId, sizeof(int64) );
#endif
	if ( !successed )
	{
		AutoLock lock( &m_connectsMutex );
		ConnectList::iterator itNetConnect = m_connectList.find( connectId );
		if ( itNetConnect == m_connectList.end() ) return 0;//�ײ��Ѿ������Ͽ�
		CloseConnect( itNetConnect );
		pConnect->Release();
		return 0;
	}

	m_pNetServer->OnConnect( pConnect->m_host );
	/*
		��������
		�������OnConnectҵ����ɣ��ſ��Կ�ʼ���������ϵ�IO�¼�
		���򣬿���ҵ�����δ������ӳ�ʼ�����������յ�OnMsg֪ͨ��
		����ҵ��㲻֪������δ�����Ϣ
		
		����δ���������pConnect���󲻴��ڲ����̷߳���
		���OnConnectҵ���У�û�йر����ӣ����ܼ������

		����������m_bConnect����AddRecv�п��ܳɹ�������OnData�л��ᴥ����
		��ΪCloseConnect����ֻ�������˹ر����ӵı�־������NetConnect�������б�ɾ����
		��û����Ĺر�socket��
		����Ϊ�˱�֤socket�����NetServer::OnCloseҵ�����ǰ������ϵͳ�ظ�ʹ�ã�

		�����ر�����NetEngine::CloseWorker()��������һ���߳��ˡ�
		�������OnConnectҵ���е����˹رգ�����CloseWorker�߳�ִ��ǰ��
		��������Ȼ�п����ȱ�ִ�У������ɹ�������������ǲ�ϣ��������
	*/
	if ( pConnect->m_bConnect )
	{
#ifdef WIN32
		IOCP_DATA iocpData;
		iocpData.connectId = connectId;
		iocpData.buf = (char*)(pConnect->PrepareBuffer(BUFBLOCK_SIZE)); 
		iocpData.bufSize = BUFBLOCK_SIZE; 
		successed = m_pNetMonitor->AddRecv( pConnect->GetSocket()->GetSocket(), (char*)&iocpData, sizeof(IOCP_DATA) );
#else
		successed = m_pNetMonitor->AddDataMonitor( pConnect->GetSocket()->GetSocket(), (char*)&connectId, sizeof(int64) );
#endif
		if ( !successed )
		{
			AutoLock lock( &m_connectsMutex );
			ConnectList::iterator itNetConnect = m_connectList.find( connectId );
			if ( itNetConnect == m_connectList.end() ) return 0;//�ײ��Ѿ������Ͽ�
			CloseConnect( itNetConnect );
		}
	}
	pConnect->Release();
	return 0;
}

void NetEngine::OnClose( int64 connectId )
{
	AutoLock lock( &m_connectsMutex );
	ConnectList::iterator itNetConnect = m_connectList.find(connectId);
	if ( itNetConnect == m_connectList.end() )return;//�ײ��Ѿ������Ͽ�
	CloseConnect( itNetConnect );
	lock.Unlock();
}

void* NetEngine::CloseWorker( NetConnect *pConnect )
{
	SetServerClose(pConnect);//���ӵķ���Ͽ�
	m_pNetServer->OnCloseConnect( pConnect->m_host );
	/*
		����pConnect->GetSocket()->Close();����
		��V1.51���У���CloseConnect( ConnectList::iterator it )���ƶ�����
		�Ƴ�ִ��close

		ȷ��ҵ������closeҵ���ϵͳ�ſ���������socket���
		��ϸԭ�򣬲ο�CloseConnect( ConnectList::iterator it )��ע��
	*/
	pConnect->GetSocket()->Close();
	pConnect->Release();//ʹ������ͷŹ������
	return 0;
}

connectState NetEngine::OnData( int64 connectId, char *pData, unsigned short uSize )
{
	connectState cs = unconnect;
	AutoLock lock( &m_connectsMutex );
	ConnectList::iterator itNetConnect = m_connectList.find(connectId);//client�б������
	if ( itNetConnect == m_connectList.end() ) return cs;//�ײ��Ѿ��Ͽ�

	NetConnect *pConnect = itNetConnect->second;
	pConnect->RefreshHeart();
	AtomAdd(&pConnect->m_useCount, 1);//ҵ����Ȼ�ȡ����
	lock.Unlock();//ȷ��ҵ���ռ�ж����HeartMonitor()���л�����pConnect��״̬
	try
	{
		cs = RecvData( pConnect, pData, uSize );//������ʵ��
		if ( unconnect == cs )
		{
			pConnect->Release();//ʹ������ͷŹ������
			OnClose( connectId );
			return cs;
		}
		/*
			���Ⲣ��MsgWorker��Ҳ���Ǳ��Ⲣ����

			��MsgWorker�Ĳ����������
			���1��MsgWorker�Ѿ�return
				��ôAtomAdd����0�������µ�MsgWorker��δ����

			���2��MsgWorkerδ��ɣ���2�����
				���1��������AtomAdd
				��Ȼ���ط�0����Ϊû�з�����AtomDec
				��������MsgWorker
				��©OnMsg��
				���ᣡ��ô��MsgWorker()��AtomAdd���ط�0������AtomDec��Ȼ����>1��
				MsgWorker()����ѭ��һ��OnMsg
				û����©OnMsg���޲���
			���2��MsgWorker��AtomDec
				��Ȼ����1����ΪMsgWorkerѭ������������1�����м���û��AtomAdd����
				MsgWorker�˳�ѭ��
				Ȼ������AtomAdd����Ȼ����0�������µ�MsgWorker��δ����
		 */
		if ( 0 < AtomAdd(&pConnect->m_nReadCount, 1) ) 
		{
			pConnect->Release();//ʹ������ͷŹ������
			return cs;
		}
		//ִ��ҵ��NetServer::OnMsg();
		m_workThreads.Accept( Executor::Bind(&NetEngine::MsgWorker), this, pConnect);
	}catch( ... ){}
	return cs;
}

void* NetEngine::MsgWorker( NetConnect *pConnect )
{
	for ( ; !m_stop; )
	{
		if ( !pConnect->m_bConnect ) 
		{
			pConnect->m_nReadCount = 0;
			break;
		}
		pConnect->m_nReadCount = 1;
		m_pNetServer->OnMsg( pConnect->m_host );//�޷���ֵ���������߼������ڿͻ�ʵ��
		if ( pConnect->IsReadAble() ) continue;
		if ( 1 == AtomDec(&pConnect->m_nReadCount,1) ) break;//����©����
	}
	//����OnClose(),ȷ��NetServer::OnClose()һ��������NetServer::OnMsg()���֮��
	if ( !pConnect->m_bConnect ) NotifyOnClose(pConnect);
	pConnect->Release();//ʹ������ͷŹ������
	return 0;
}

connectState NetEngine::RecvData( NetConnect *pConnect, char *pData, unsigned short uSize )
{
	return unconnect;
}

//�ر�һ������
void NetEngine::CloseConnect( int64 connectId )
{
	AutoLock lock( &m_connectsMutex );
	ConnectList::iterator itNetConnect = m_connectList.find( connectId );
	if ( itNetConnect == m_connectList.end() ) return;//�ײ��Ѿ������Ͽ�
	CloseConnect( itNetConnect );
}

//��Ӧ��������¼�
connectState NetEngine::OnSend( int64 connectId, unsigned short uSize )
{
	connectState cs = unconnect;
	AutoLock lock( &m_connectsMutex );
	ConnectList::iterator itNetConnect = m_connectList.find(connectId);
	if ( itNetConnect == m_connectList.end() )return cs;//�ײ��Ѿ������Ͽ�
	NetConnect *pConnect = itNetConnect->second;
	AtomAdd(&pConnect->m_useCount, 1);//ҵ����Ȼ�ȡ����
	lock.Unlock();//ȷ��ҵ���ռ�ж����HeartMonitor()���л�����pConnect��״̬
	try
	{
		if ( pConnect->m_bConnect ) cs = SendData(pConnect, uSize);
	}
	catch(...)
	{
	}
	pConnect->Release();//ʹ������ͷŹ������
	return cs;
	
}

connectState NetEngine::SendData(NetConnect *pConnect, unsigned short uSize)
{
	return unconnect;
}

bool NetEngine::Listen(int port)
{
	AutoLock lock(&m_listenMutex);
	pair<map<int,int>::iterator,bool> ret 
		= m_serverPorts.insert(map<int,int>::value_type(port,INVALID_SOCKET));
	map<int,int>::iterator it = ret.first;
	if ( !ret.second && INVALID_SOCKET != it->second ) return true;
	if ( m_stop ) return true;

	it->second = ListenPort(port);
	if ( INVALID_SOCKET == it->second ) return false;
	return true;
}

int NetEngine::ListenPort(int port)
{
	return INVALID_SOCKET;
}

bool NetEngine::ListenAll()
{
	bool ret = true;
	AutoLock lock(&m_listenMutex);
	map<int,int>::iterator it = m_serverPorts.begin();
	char strPort[256];
	string strFaild;
	for ( ; it != m_serverPorts.end(); it++ )
	{
		if ( INVALID_SOCKET != it->second ) continue;
		it->second = ListenPort(it->first);
		if ( INVALID_SOCKET == it->second ) 
		{
			sprintf( strPort, "%d", it->first );
			strFaild += strPort;
			strFaild += " ";
			ret = false;
		}
	}
	if ( !ret ) m_startError += "listen port:" + strFaild + "faild";
	return ret;
}

bool NetEngine::Connect(const char* ip, int port, void *pSvrInfo, int reConnectTime)
{
	uint64 addr64 = 0;
	if ( !addrToI64(addr64, ip, port) ) return false;
	
	AutoLock lock(&m_serListMutex);
	
	vector<SVR_CONNECT*> sockArray;
	map<uint64,vector<SVR_CONNECT*> >::iterator it = m_keepIPList.find(addr64);
	if ( it == m_keepIPList.end() ) m_keepIPList.insert( map<uint64,vector<SVR_CONNECT*> >::value_type(addr64,sockArray) );
	SVR_CONNECT *pSvr = new SVR_CONNECT;
	pSvr->reConnectSecond = reConnectTime;
	pSvr->lastConnect = 0;
	pSvr->sock = INVALID_SOCKET;
	pSvr->addr = addr64;
	pSvr->state = SVR_CONNECT::unconnected;
	pSvr->pSvrInfo = pSvrInfo;
	m_keepIPList[addr64].push_back(pSvr);
	if ( m_stop ) return false;
	
	//�������ӽ��
	pSvr->lastConnect = time(NULL);
	ConnectResult ret = ConnectOtherServer(ip, port, pSvr->sock);
	if ( NetEngine::success == ret )
	{
		OnConnect(pSvr->sock, pSvr->sock, pSvr);
		pSvr->state = SVR_CONNECT::connected;
	}
	else if ( NetEngine::waitReulst == ret )
	{
		pSvr->state = SVR_CONNECT::connectting;
		m_wakeConnectThread.Notify();
	}
	else //����ʧ��
	{
		pSvr->state = SVR_CONNECT::unconnectting;
		m_workThreads.Accept( Executor::Bind(&NetEngine::ConnectFailed), this, pSvr );
	}
	
	return true;
}

NetEngine::ConnectResult NetEngine::ConnectOtherServer(const char* ip, int port, int &svrSock)
{
	svrSock = INVALID_SOCKET;
	Socket sock;//����socket
	if ( !sock.Init( Socket::tcp ) ) return NetEngine::cannotCreateSocket;
	sock.SetSockMode();
// 	bool successed = sock.Connect(ip, port);
	svrSock = sock.Detach();
	return AsycConnect(svrSock, ip, port);
}

bool NetEngine::ConnectAll()
{
	if ( m_stop ) return false;
	AutoLock lock(&m_serListMutex);
	time_t curTime = time(NULL);
	char ip[24];
	int port;
	int i = 0;
	int count = 0;
	int sock = INVALID_SOCKET;
	
	//��������
	SVR_CONNECT *pSvr = NULL;
	map<uint64,vector<SVR_CONNECT*> >::iterator it = m_keepIPList.begin();
	vector<SVR_CONNECT*>::iterator itSvr;
	for ( ; it != m_keepIPList.end(); it++ )
	{
		i64ToAddr(ip, port, it->first);
		itSvr = it->second.begin();
		for ( ; itSvr != it->second.end();  )
		{
			pSvr = *itSvr;
			if ( SVR_CONNECT::connectting == pSvr->state 
				|| SVR_CONNECT::connected == pSvr->state 
				|| SVR_CONNECT::unconnectting == pSvr->state 
				) 
			{
				if ( 0 > pSvr->reConnectSecond && SVR_CONNECT::connected == pSvr->state ) //�����ӳɹ��ģ��ͷŲ���Ҫ������
				{
					itSvr = it->second.erase(itSvr);
					delete pSvr;
					continue;
				}
				itSvr++;
				continue;
			}
			if ( 0 > pSvr->reConnectSecond && 0 != pSvr->lastConnect ) 
			{
				itSvr = it->second.erase(itSvr);
				delete pSvr;
				continue;
			}
			if ( curTime - pSvr->lastConnect < pSvr->reConnectSecond ) 
			{
				itSvr++;
				continue;
			}
			
			pSvr->lastConnect = curTime;
			ConnectResult ret = ConnectOtherServer(ip, port, pSvr->sock);
			if ( NetEngine::success == ret )
			{
				OnConnect(pSvr->sock, pSvr->sock, pSvr);
				pSvr->state = SVR_CONNECT::connected;
			}
			else if ( NetEngine::waitReulst == ret )
			{
				pSvr->state = SVR_CONNECT::connectting;
				m_wakeConnectThread.Notify();
			}
			else //����ʧ��
			{
				pSvr->state = SVR_CONNECT::unconnectting;
				m_workThreads.Accept( Executor::Bind(&NetEngine::ConnectFailed), this, pSvr );
			}
			itSvr++;
		}
	}
	
	return true;
}

void NetEngine::SetServerClose(NetConnect *pConnect)
{
	if ( !pConnect->m_host.IsServer() ) return;
	int sock = pConnect->GetSocket()->GetSocket();
	AutoLock lock(&m_serListMutex);
	map<uint64,vector<SVR_CONNECT*> >::iterator it = m_keepIPList.begin();
	int i = 0;
	int count = 0;
	SVR_CONNECT *pSvr = NULL;
	for ( ; it != m_keepIPList.end(); it++ )
	{
		count = it->second.size();
		for ( i = 0; i < count; i++ )
		{
			pSvr = it->second[i];
			if ( sock != pSvr->sock ) continue;
			pSvr->sock = INVALID_SOCKET;
			pSvr->state = SVR_CONNECT::unconnected;
			return;
		}
	}
}

//��ĳ�����ӹ㲥��Ϣ(ҵ���ӿ�)
void NetEngine::BroadcastMsg( int *recvGroupIDs, int recvCount, char *msg, unsigned int msgsize, int *filterGroupIDs, int filterCount )
{
	//////////////////////////////////////////////////////////////////////////
	//�ر�������������
	ConnectList::iterator it;
	NetConnect *pConnect;
	vector<NetConnect*> recverList;
	//���������й㲥�������Ӹ��Ƶ�һ��������
	AutoLock lock( &m_connectsMutex );
	for ( it = m_connectList.begin(); it != m_connectList.end(); it++ )
	{
		pConnect = it->second;
		if ( !pConnect->IsInGroups(recvGroupIDs, recvCount) 
			|| pConnect->IsInGroups(filterGroupIDs, filterCount) ) continue;
		recverList.push_back(pConnect);
		AtomAdd(&pConnect->m_useCount, 1);//ҵ����Ȼ�ȡ����
	}
	lock.Unlock();
	
	//������е����ӿ�ʼ�㲥
	vector<NetConnect*>::iterator itv = recverList.begin();
	for ( ; itv != recverList.end(); itv++ )
	{
		pConnect = *itv;
		if ( pConnect->m_bConnect ) pConnect->SendData((const unsigned char*)msg,msgsize);
		pConnect->Release();//ʹ������ͷŹ������
	}
}

//��ĳ����������Ϣ(ҵ���ӿ�)
bool NetEngine::SendMsg( int64 hostID, char *msg, unsigned int msgsize )
{
	AutoLock lock( &m_connectsMutex );
	ConnectList::iterator itNetConnect = m_connectList.find(hostID);
	if ( itNetConnect == m_connectList.end() ) return false;//�ײ��Ѿ������Ͽ�
	NetConnect *pConnect = itNetConnect->second;
	AtomAdd(&pConnect->m_useCount, 1);//ҵ����Ȼ�ȡ����
	lock.Unlock();
	bool ret = false;
	if ( pConnect->m_bConnect ) ret = pConnect->SendData((const unsigned char*)msg,msgsize);
	pConnect->Release();//ʹ������ͷŹ������

	return ret;
}

const char* NetEngine::GetInitError()//ȡ������������Ϣ
{
	return m_startError.c_str();
}

void* NetEngine::ConnectThread(void*)
{
	SVR_CONNECT** clientList = (SVR_CONNECT**)new mdk::uint64[20000];
	int clientCount = 0;
	int i = 0;
	SVR_CONNECT *pSvr = NULL;
#ifndef WIN32
	m_hEPoll = epoll_create(20000);
	mdk_assert( -1 != m_hEPoll );
	m_events = new epoll_event[20000];	//epoll�¼�
	mdk_assert( NULL != m_events );
#endif

	bool isEnd = true;//�Ѿ�����m_keepIPListĩβ
	while ( !m_stop )
	{
		//��������connectting״̬��sock�������б�
		clientCount = 0;
		isEnd = true;
		{
			AutoLock lock(&m_serListMutex);
			map<uint64,vector<SVR_CONNECT*> >::iterator it = m_keepIPList.begin();
			for ( ; it != m_keepIPList.end() && clientCount < 20000; it++ )
			{
				for ( i = 0; i < it->second.size() && clientCount < 20000; i++ )
				{
					pSvr = it->second[i];
					if ( SVR_CONNECT::connectting != pSvr->state ) continue;
					mdk_assert( INVALID_SOCKET != pSvr->sock );
					clientList[clientCount++] = pSvr;
				}
			}
			if ( it != m_keepIPList.end() ) isEnd = false;
		}
#ifndef WIN32
		bool finished = EpollConnect( clientList, clientCount );
#else
		bool finished = SelectConnect( clientList, clientCount );
#endif
		/*
			��m_keepIPList���ó������������Ӷ��Ѿ����ؽ������m_keepIPList��û�������ڵȴ�����
			��ȴ��µ��������񱻷���m_keepIPList
		*/
		if ( finished && isEnd ) m_wakeConnectThread.Wait();
	}
	uint64 *p = (uint64*)clientList;
	delete[]p;
#ifndef WIN32
	delete[]m_events;
#endif
	return NULL;
}

#ifndef WIN32
#include <netdb.h>
#endif

NetEngine::ConnectResult NetEngine::AsycConnect( int svrSock, const char *lpszHostAddress, unsigned short nHostPort )
{
	if ( NULL == lpszHostAddress ) return NetEngine::invalidParam;
	//������ת��Ϊ��ʵIP�����lpszHostAddress��������ip����Ӱ��ת�����
	char ip[64]; //��ʵIP
#ifdef WIN32
	PHOSTENT hostinfo;   
#else
	struct hostent * hostinfo;   
#endif
	strcpy( ip, lpszHostAddress ); 
	if((hostinfo = gethostbyname(lpszHostAddress)) != NULL)   
	{
		strcpy( ip, inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list) ); 
	}

	//ʹ����ʵip��������
	sockaddr_in sockAddr;
	memset(&sockAddr,0,sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(ip);
	sockAddr.sin_port = htons( nHostPort );

	if ( SOCKET_ERROR != connect(svrSock, (sockaddr*)&sockAddr, sizeof(sockAddr)) ) 
	{
		return NetEngine::success;
	}
#ifndef WIN32
	if ( EINPROGRESS == errno ) 
	{
		return NetEngine::waitReulst;
	}
#else
	int nError = GetLastError();
	if ( WSAEWOULDBLOCK == nError ) 
	{
		return NetEngine::waitReulst;
	}
#endif

	return NetEngine::faild;
}

void* NetEngine::ConnectFailed( NetEngine::SVR_CONNECT *pSvr )
{
	if ( NULL == pSvr ) return NULL;
	char ip[32];
	int port;
	int reConnectSecond;
	i64ToAddr(ip, port, pSvr->addr);
	reConnectSecond = pSvr->reConnectSecond;
	int svrSock = pSvr->sock;
	pSvr->sock = INVALID_SOCKET;
	pSvr->state = SVR_CONNECT::unconnected;
	closesocket(svrSock);

	m_pNetServer->OnConnectFailed( ip, port, reConnectSecond );

	return NULL;
}

bool NetEngine::EpollConnect( SVR_CONNECT **clientList, int clientCount )
{
	bool finished = true;
#ifndef WIN32
	int i = 0;
	SVR_CONNECT *pSvr = NULL;
	epoll_event ev;
	int monitorCount = 0;


	//////////////////////////////////////////////////////////////////////////
	//ȫ�������������
	for ( i = 0; i < clientCount; i++ )
	{
		pSvr = clientList[i];
		ev.events = EPOLLIN|EPOLLOUT|EPOLLET;
		ev.data.ptr = pSvr;
		if ( 0 > epoll_ctl(m_hEPoll, EPOLL_CTL_ADD, pSvr->sock, &ev) ) pSvr->inEpoll = false;
		else 
		{
			pSvr->inEpoll = true;
			monitorCount++;
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//ѭ��epoll_wait()ֱ����ʵʧ��or��ʱor�����¼�
	int count = 0;
	while ( true )
	{
		count = epoll_wait(m_hEPoll, m_events, 20000, 20000 );
		if ( -1 == count ) 
		{
			if ( EINTR == errno ) continue;//��ʧ��
		}
		break;
	}
	if ( -1 == count ) printf( "epoll_wait return %d\n", count );
	volatile int errCode = errno;//epoll_waitʧ��ʱ״̬

	/*
		����A:�������ڹ���Bִ��
		��Ϊ����B�����������ʧ�ܣ��ὫclientList[i]->sock����ΪINVALID_SOCKET
		����A��������epoll�����ж�ʱ����clientList�о��Ѿ��Ҳ�����������
	*/
	//////////////////////////////////////////////////////////////////////////
	//����A:���epoll�������У�����ȫ��©��֪ͨ
	/*
		������������
		�����м����еľ����epoll��������ɾ��
		�����з���epoll��������ʧ�ܵľ������Ϊ�ǿɶ���д�ģ�ȥ����һ�������Ƿ�ɹ�
	*/
	errCode = errno;//epoll_waitʧ��ʱ״̬
	volatile int delSock = 0;
	volatile int delError = 0;
	for ( i = 0; i < clientCount; i++ )
	{
		if ( clientList[i]->inEpoll )
		{
			delSock = clientList[i]->sock;
			clientList[i]->inEpoll = false;
			delError = epoll_ctl(m_hEPoll, EPOLL_CTL_DEL, clientList[i]->sock, NULL); 
			mdk_assert( 0 == delError );//��Ӧ��ɾ��ʧ�ܣ����ʧ��ǿ�Ʊ���
			if ( 0 >= count ) ConnectIsFinished(clientList[i], true, true, count, errCode );//��������epoll_ctlɾ����ԭ��ͬ����A����Bִ��˳��
		}
		else//��ӵ�Epollʧ�ܵľ������Ϊ�ɶ���д�����볢��ȡIP�ȷ���
		{
			ConnectIsFinished(clientList[i], true, true, 1, errCode );
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//����B:�����¼�
	if ( 0 < count ) //����epoll֪ͨ���
	{
		if ( count < monitorCount ) finished = false;//������δ���ؽ����sock
		for ( i = 0; i < count; i++ )
		{
			pSvr = (SVR_CONNECT*)(m_events[i].data.ptr);
			if ( !ConnectIsFinished(pSvr, m_events[i].events&EPOLLIN, m_events[i].events&EPOLLOUT, count, errCode ) )//sock��δ���ؽ��
			{
				finished = false;
			}
		}
	}


#endif
	return finished;
}

bool NetEngine::SelectConnect( SVR_CONNECT **clientList, int clientCount )
{
	bool finished = true;
#ifdef WIN32
	int startPos = 0;
	int endPos = 0;
	int i = 0;
	int maxSocket = 0;
	SVR_CONNECT *pSvr = NULL;
	fd_set readfds; 
	fd_set sendfds; 


	//��ʼ������ÿ�μ���1000��sock,select1��������1024��
	int svrSock;
	for ( endPos = 0; endPos < clientCount; )
	{
		maxSocket = 0;
		FD_ZERO(&readfds);     
		FD_ZERO(&sendfds);  
		startPos = endPos;//��¼���μ���sock��ʼλ��
		for ( i = 0; i < FD_SETSIZE - 1 && endPos < clientCount; i++ )
		{
			pSvr = clientList[endPos];
			if ( maxSocket < pSvr->sock ) maxSocket = pSvr->sock;
			FD_SET(pSvr->sock, &readfds); 
			FD_SET(pSvr->sock, &sendfds); 
			endPos++;
		}

		//��ʱ����
		timeval outtime;
		outtime.tv_sec = 20;
		outtime.tv_usec = 0;
		int nSelectRet =::select( maxSocket + 1, &readfds, &sendfds, NULL, &outtime ); //����д״̬
		int errCode = GetLastError();
		if ( SOCKET_ERROR == nSelectRet ) //����ʱ������ʹ�ӡ���о��״̬
		{
			printf( "select return %d\n", nSelectRet );
			for ( i = startPos; i < endPos; i++ )
			{
				pSvr = clientList[i];
				svrSock = pSvr->sock;
				printf( "select = %d errno(%d) sock(%d) read(%d) send(%d)\n", nSelectRet, errCode, svrSock, FD_ISSET(svrSock, &readfds), FD_ISSET(svrSock, &sendfds) );
			}
		}

		for ( i = startPos; i < endPos; i++ )
		{
			if ( !ConnectIsFinished(clientList[i], 
				0 != FD_ISSET(clientList[i]->sock, &readfds), 
				0 != FD_ISSET(clientList[i]->sock, &sendfds), 
				nSelectRet, errCode ) )//������δ���ؽ��,���������󲻵ȴ�����������δ���ص�sock
			{
				finished = false;
			}
		}
	}
#endif
	return finished;
}

bool NetEngine::ConnectIsFinished( SVR_CONNECT *pSvr, bool readable, bool sendable, int api, int errorCode )
{
	int reason = 0;
	bool successed = true;
	int nSendSize0 = 0;
	int sendError0 = 0;
	int nSendSize1 = 0;
	int sendError1 = 0;
	char buf[256];
	int nRecvSize0 = 0;
	int recvError0 = 0;
	int nRecvSize1 = 0;
	int recvError1 = 0;
	char clientIP[256];
	char serverIP[256];
	int svrSock = pSvr->sock;

	if ( sendable )
	{
		nSendSize0 = send(svrSock, buf, 0, 0);
		sendError0 = 0;
		if ( 0 > nSendSize0 ) 
		{
#ifdef WIN32
			sendError0 = GetLastError();
#else
			sendError0 = errno;
#endif
			successed = false;
		}
	}
	if ( readable )
	{
		nRecvSize0 = recv(svrSock, buf, 0, MSG_PEEK);
		recvError0 = 0;
		if ( SOCKET_ERROR == nRecvSize0 )
		{
#ifdef WIN32
			recvError0 = GetLastError();
#else
			recvError0 = errno;
#endif
			successed = false;
		}

		nRecvSize1 = recv(svrSock, buf, 1, MSG_PEEK);
		recvError1 = 0;
		if ( SOCKET_ERROR == nRecvSize1 )
		{
#ifdef WIN32
			recvError1 = GetLastError();
#else
			recvError1 = errno;
#endif
			successed = false;
		}
	}

	if ( 0 >= api ) 
	{
		successed = false;
		reason = 1;
	}
	else if ( successed )
	{
		if ( !readable && !sendable ) //sock��δ���ؽ��
		{
			return false;
		}
		sockaddr_in sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		socklen_t nSockAddrLen = sizeof(sockAddr);
		if ( SOCKET_ERROR == getsockname( svrSock, (sockaddr*)&sockAddr, &nSockAddrLen ) ) 
		{
			reason = 2;
			successed = false;
		}
		else
		{
			strcpy(clientIP, inet_ntoa(sockAddr.sin_addr));

			if ( 0 == strcmp("0.0.0.0", clientIP) ) 
			{
				reason = 3;
				successed = false;
			}
			else
			{
				memset(&sockAddr, 0, sizeof(sockAddr));
				nSockAddrLen = sizeof(sockAddr);
				if ( SOCKET_ERROR == getpeername( svrSock, (sockaddr*)&sockAddr, &nSockAddrLen ) ) 
				{
					reason = 4;
					successed = false;
				}
				else strcpy(serverIP, inet_ntoa(sockAddr.sin_addr));
			}
		}
	}
	if ( !successed )
	{
		pSvr->state = SVR_CONNECT::unconnectting;
		m_workThreads.Accept( Executor::Bind(&NetEngine::ConnectFailed), this, pSvr );
		return true;
	}

	OnConnect(svrSock, svrSock, pSvr);
	pSvr->state = SVR_CONNECT::connected;
	return true;
}

//��TCP_NODELAY
void NetEngine::OpenNoDelay()
{
	m_noDelay = true;
}

}
// namespace mdk

