#ifndef MDK_ST_NET_ENGINE_H
#define MDK_ST_NET_ENGINE_H

#include "../../../include/mdk/Socket.h"
#include "../../../include/mdk/FixLengthInt.h"
#include "../../../include/mdk/MemoryPool.h"
#include "../../../include/mdk/Thread.h"
#include "../../../include/mdk/Lock.h"

#include <map>
#include <vector>
#include <string>
#ifndef WIN32
#include <sys/epoll.h>
#endif

namespace mdk
{
class STNetConnect;
class NetHost;
class NetEventMonitor;
class STIocp;
class STEpoll;
class STNetServer;
class MemoryPool;
typedef std::map<int,STNetConnect*> ConnectList;
	
/**
 * ������ͨ��������(���̰߳�)
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
class STNetEngine
{
	friend class STNetServer;
protected:
	std::string m_startError;//����ʧ��ԭ��
	MemoryPool *m_pConnectPool;//STNetConnect�����
	int m_averageConnectCount;//ƽ��������
	bool m_stop;//ֹͣ��־
	/**
		���ӱ�
		map<unsigned long,STNetConnect*>
		��ʱ�����б����������з���������
		��û�����������ӶϿ�
	*/
	ConnectList m_connectList;
	int m_nHeartTime;//�������(S)
	Thread m_mainThread;
#ifdef WIN32
	STIocp *m_pNetMonitor;
#else
	STEpoll *m_pNetMonitor;
	std::map<int,int> m_ioList;//δ���io������socket�б�
#endif
	STNetServer *m_pNetServer;
	std::map<int,int> m_serverPorts;//�ṩ����Ķ˿�,key�˿ڣ�value״̬��������˿ڵ��׽���
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
protected:
	//win������io����
	bool WINIO(int timeout);
	//linux������io����
	bool LinuxIO(int timeout);
	//��Ӧ�����¼�,sockΪ�����ӵ��׽���
	bool OnConnect( int sock, int listenSock, SVR_CONNECT *pSvr = NULL );
	//��Ӧ�ر��¼���sockΪ�رյ��׽���
	void OnClose( int sock );
	void NotifyOnClose(STNetConnect *pConnect);//����OnClose֪ͨ
	//��Ӧ���ݵ����¼���sockΪ�����ݵ�����׽���
	connectState OnData( int sock, char *pData, unsigned short uSize );
	/*
		��������
		��������״̬
	*/
	connectState RecvData( STNetConnect *pConnect, char *pData, unsigned short uSize );
	void* MsgWorker( STNetConnect *pConnect );//ҵ��㴦����Ϣ
	connectState OnSend( int sock, unsigned short uSize );//��Ӧ�����¼�
	virtual connectState SendData(STNetConnect *pConnect, unsigned short uSize);//��������
	virtual int ListenPort(int port);//����һ���˿�,���ش������׽���
	//��ĳ�����ӹ㲥��Ϣ(ҵ���ӿ�)
	void BroadcastMsg( int *recvGroupIDs, int recvCount, char *msg, unsigned int msgsize, int *filterGroupIDs, int filterCount );
	void SendMsg( int hostID, char *msg, unsigned int msgsize );//��ĳ����������Ϣ(ҵ���ӿ�)
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
	STNetEngine::ConnectResult ConnectOtherServer(const char* ip, int port, int &svrSock);//�첽����һ������,���̳ɹ�����true�����򷵻�false���ȴ�select���
	bool ConnectAll();//��������ע��ķ��������ӵĻ��Զ�����
	void SetServerClose(STNetConnect *pConnect);//���������ӵķ���Ϊ�ر�״̬
	const char* GetInitError();//ȡ������������Ϣ
	void Select();//������ⷢ�����ӵĽ��
	void* ConnectFailed( STNetEngine::SVR_CONNECT *pSvr );
	STNetEngine::ConnectResult AsycConnect( int svrSock, const char *lpszHostAddress, unsigned short nHostPort );
	bool EpollConnect( SVR_CONNECT **clientList, int clientCount );//clientCount������ȫ���յ����������true,���򷵻�false
	bool SelectConnect( SVR_CONNECT **clientList, int clientCount );//clientCount������ȫ���յ����������true,���򷵻�false
	bool ConnectIsFinished( SVR_CONNECT *pSvr, bool readable, bool sendable, int api, int errorCode );//��������ɷ���true������ʾ�ɹ���ʧ��Ҳ����ɣ��ⲿ����Ҫ�����ɹ�ʧ�����ڲ��Ѵ��������򷵻�false
public:
	/**
	 * ���캯��,�󶨷�������ͨ�Ų���
	 * 
	 */
	STNetEngine();
	virtual ~STNetEngine();

	//����ƽ��������
	void SetAverageConnectCount(int count);
	//��������ʱ��
	void SetHeartTime( int nSecond );
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
	void CloseConnect( int sock );
	//����һ���˿�
	bool Listen( int port );
	/*
		����һ������
		reConnectTime < 0��ʾ�Ͽ��������Զ�����
	*/
	bool Connect(const char* ip, int port, void *pSvrInfo, int reConnectTime);
	SVR_CONNECT** m_clientList;
#ifndef WIN32
	int m_hEPoll;
	epoll_event *m_events;
#endif
	bool m_noDelay;//����TCP_NODELAY
};

}  // namespace mdk
#endif //MDK_ST_NET_ENGINE_H
