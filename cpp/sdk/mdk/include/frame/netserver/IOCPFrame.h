// IOCPFrame.h: interface for the IOCPFrame class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MDK_IOCPFRAME_H
#define MDK_IOCPFRAME_H

#include "NetEngine.h"

namespace mdk
{

class IOCPFrame : public NetEngine  
{
public:
	IOCPFrame();
	virtual ~IOCPFrame();

protected:
	//�����¼������߳�
	void* NetMonitor( void* );
	//�������ݣ����ӹر�ʱ����false���������Ӧ�������Ҫ��������ʵ��
	connectState RecvData( NetConnect *pConnect, char *pData, unsigned short uSize );
	//��������
	connectState SendData(NetConnect *pConnect, unsigned short uSize);
	int ListenPort(int port);//����һ���˿�,���ش������׽���
};

}//namespace mdk

#endif // MDK_IOCPFRAME_H
