// EpollFrame.h: interface for the EpollFrame class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MDK_EPOLLFRAME_H
#define MDK_EPOLLFRAME_H

#include "NetEngine.h"

namespace mdk
{
class EpollFrame : public NetEngine  
{
public:
	EpollFrame();
	virtual ~EpollFrame();
	
protected:
	//�����¼������߳�
	void* NetMonitor( void* );
	//�������ݣ����ӹر�ʱ����false���������Ӧ�������Ҫ��������ʵ��
	connectState RecvData( NetConnect *pConnect, char *pData, unsigned short uSize );
	//��������
	connectState SendData(NetConnect *pConnect, unsigned short uSize);
	int ListenPort(int port);//����һ���˿�,���ش������׽���

	void NewConnectMonitor();
	void DataMonitor();
	void SendAbleMonitor();

public:
};

}//namespace mdk

#endif // MDK_EPOLLFRAME_H
