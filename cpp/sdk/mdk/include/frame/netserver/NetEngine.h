#ifndef MDK_NET_ENGINE_H
#define MDK_NET_ENGINE_H

#include "../../../include/mdk/Socket.h"
#include "../../../include/mdk/Thread.h"
#include "../../../include/mdk/Lock.h"
#include "../../../include/mdk/ThreadPool.h"
#include "../../../include/mdk/FixLengthInt.h"
#include "../../../include/mdk/MemoryPool.h"
#include "../../../include/mdk/Signal.h"

#include <map>
#include <vector>
#include <string>
#ifndef WIN32
#include <sys/epoll.h>
#endif

namespace mdk
{
class Mutex;
class NetConnect;
class NetHost;
class NetEventMonitor;
class NetServer;
class MemoryPool;
typedef std::map<int64,NetConnect*> ConnectList;
/**
 * ������ͨ��������
 * ͨ�Ų��������
 * ʹ��һ��ͨ�Ų��ԣ�IOCP��EPoll��ͳ��select�ȣ�����ͨ�ſ���
 * 
 * ��Ա
 * �������������
 * �ͻ����������ӳ���
 * �ӿ�
 * ����
 * ֹͣ
 * 
 * ����
 * ����������
 * �Ͽ�����
 * ��Ϣ����
 * 
 */
enum connectState
{
	ok = 0,
	unconnect = 1,
	wait_recv = 2,
	wait_send = 3,
};
class NetEngine
{
	friend class NetServer;
protected:
	std::string m_startError;//����ʧ��ԭ��
	MemoryPool *m_pConnectPool;//NetConnect�����
	int m_averageConnectCount;//ƽ��������
	bool m_stop;//ֹͣ��־
	Signal m_sigStop;//ֹͣ�ź�
	/**
		���ӱ�
		map<unsigned long,NetConnect*>
		��ʱ�����б����������з���������
		��û�����������ӶϿ�
	*/
	ConnectList m_connectList;
	Mutex m_connectsMutex;//�����б���ʿ���
	int m_nHeartTime;//�������(S)
	Thread m_mainThread;
	NetEventMonitor *m_pNetMonitor;
	ThreadPool m_ioThreads;//io�̳߳�
	int m_ioThreadCount;//io�߳�����
	ThreadPool m_workThreads;//ҵ���̳߳�
	int m_workThreadCount;//ҵ���߳�����
	NetServer *m_pNetServer;
	std::map<int,int> m_serverPorts;//�ṩ����Ķ˿�,key�˿ڣ�value״̬��������˿ڵ��׽���
	Mutex m_listenMutex;//������������

	typedef struct SVR_CONNECT
	{
		enum ConnectState
		{
			unconnected = 0,
				connectting = 1,
				unconnectting = 2,
				connected = 3,
		};
		int sock;				//���
		uint64 addr;				//��ַ
		int reConnectSecond;		//����ʱ�䣬С��0��ʾ������
		time_t lastConnect;			//�ϴγ�������ʱ��
		ConnectState state;			//����״̬
		void *pSvrInfo;				//������Ϣ
#ifndef WIN32
		bool inEpoll;				//��epoll��
#endif
	}SVR_CONNECT;
	std::map<uint64,std::vector<SVR_CONNECT*> > m_keepIPList;//Ҫ�������ӵ��ⲿ�����ַ�б��Ͽ�������
	Mutex m_serListMutex;//���ӵķ����ַ�б���
	Thread m_connectThread;
	Signal m_wakeConnectThread;
protected:
	//�����¼������߳�
	virtual void* NetMonitor( void* ) = 0;
	void* RemoteCall NetMonitorTask( void* );
	//��Ӧ�����¼�,sockΪ�����ӵ��׽���
	bool OnConnect( int sock, int listenSock, SVR_CONNECT *pSvr = NULL );
	void* RemoteCall ConnectWorker( NetConnect *pConnect );//ҵ��㴦������
	//��Ӧ�ر��¼���sockΪ�رյ��׽���
	void OnClose( int64 connectId );
	void NotifyOnClose(NetConnect *pConnect);//����OnClose֪ͨ
	void* RemoteCall CloseWorker( NetConnect *pConnect );//ҵ��㴦��ر�
	void* RemoteCall ConnectFailed( NetEngine::SVR_CONNECT *pSvr );//ҵ��㴦��������������ʧ��
	//��Ӧ���ݵ����¼���sockΪ�����ݵ�����׽���
	connectState OnData( int64 connectId, char *pData, unsigned short uSize );
	/*
		��������
		��������״̬
		�������Ӧ�������Ҫ��������ʵ��
	*/
	virtual connectState RecvData( NetConnect *pConnect, char *pData, unsigned short uSize );
	void* RemoteCall MsgWorker( NetConnect *pConnect );//ҵ��㴦����Ϣ
	connectState OnSend( int64 connectId, unsigned short uSize );//��Ӧ�����¼�
	virtual connectState SendData(NetConnect *pConnect, unsigned short uSize);//��������
	virtual int ListenPort(int port);//����һ���˿�,���ش������׽���
	//��ĳ�����ӹ㲥��Ϣ(ҵ���ӿ�)
	void BroadcastMsg( int *recvGroupIDs, int recvCount, char *msg, unsigned int msgsize, int *filterGroupIDs, int filterCount );
	bool SendMsg( int64 hostID, char *msg, unsigned int msgsize );//��ĳ����������Ϣ(ҵ���ӿ�)
private:
	//���߳�
	void* RemoteCall Main(void*);
	//�����߳�
	void HeartMonitor();
	//�ر�һ�����ӣ���socket�Ӽ�������ɾ��
	void CloseConnect( ConnectList::iterator it );

	//////////////////////////////////////////////////////////////////////////
	//����˿�
	bool ListenAll();//��������ע��Ķ˿�
	//////////////////////////////////////////////////////////////////////////
	//����������������
	enum ConnectResult
	{
		success = 0,
		waitReulst = 1,
		cannotCreateSocket = 2,
		invalidParam = 3,
		faild = 4,
	};
	NetEngine::ConnectResult ConnectOtherServer(const char* ip, int port, int &svrSock);//�첽����һ������,���̳ɹ�����true�����򷵻�false���ȴ�select���
	bool ConnectAll();//��������ע��ķ��������ӵĻ��Զ�����
	void SetServerClose(NetConnect *pConnect);//���������ӵķ���Ϊ�ر�״̬
	const char* GetInitError();//ȡ������������Ϣ
	void* RemoteCall ConnectThread(void*);//�첽�����߳�
	NetEngine::ConnectResult AsycConnect( int svrSock, const char *lpszHostAddress, unsigned short nHostPort );
	bool EpollConnect( SVR_CONNECT **clientList, int clientCount );//clientCount������ȫ���յ����������true,���򷵻�false
	bool SelectConnect( SVR_CONNECT **clientList, int clientCount );//clientCount������ȫ���յ����������true,���򷵻�false
	bool ConnectIsFinished( SVR_CONNECT *pSvr, bool readable, bool sendable, int api, int errorCode );//��������ɷ���true������ʾ�ɹ���ʧ��Ҳ����ɣ��ⲿ����Ҫ�����ɹ�ʧ�����ڲ��Ѵ��������򷵻�false
	
public:
	/**
	 * ���캯��,�󶨷�������ͨ�Ų���
	 * 
	 */
	NetEngine();
	virtual ~NetEngine();

	//����ƽ��������
	void SetAverageConnectCount(int count);
	//��������ʱ��
	void SetHeartTime( int nSecond );
	//��������IO�߳�����
	void SetIOThreadCount(int nCount);
	//���ù����߳���
	void SetWorkThreadCount(int nCount);
	//���ù����߳������ص�����
	void SetOnWorkStart( MethodPointer method, void *pObj, void *pParam );
	void SetOnWorkStart( FuntionPointer fun, void *pParam );
	//��TCP_NODELAY
	void OpenNoDelay();
	/**
	 * ��ʼ
	 * �ɹ�����true��ʧ�ܷ���false
	 */
	bool Start();
	//ֹͣ
	void Stop();
	//�ȴ�ֹͣ
	void WaitStop();
	//�ر�һ���������,ͨ�Ų㷢���������ر�����ʱ����������ýӿ�
	void CloseConnect( int64 connectId );
	//����һ���˿�
	bool Listen( int port );
	/*
	����һ������
	reConnectTime < 0��ʾ�Ͽ��������Զ�����
	*/
	bool Connect(const char* ip, int port, void *pSvrInfo, int reConnectTime);
#ifndef WIN32
	int m_hEPoll;
	epoll_event *m_events;
#endif
	int64 m_nextConnectId;
	bool m_noDelay;//����TCP_NODELAY
};

}  // namespace mdk
#endif //MDK_NET_ENGINE_H
