#ifndef CLIENT_H
#define CLIENT_H

#include "protocl/cpp/base/Socket.h"
#include "protocl/cpp/Buffer.h"
#include <string>
#include <map>
#include <cstdlib>
#include <cstdio>

#include "common/CallResult.h"

class Client 
{
public:
	Client(void);
	virtual ~Client(void);

	void SetService( int nodeId, const std::string &ip, int port );
	/*
		�������Ƿ����
		����δ�����ķ���5�����ڲ��ټ�����״̬��ֱ����Ϊ�Ƿ���δ����
	*/
	bool IsReady();
	bool IsEnd();//���ݽ������
	void Close();
protected:
	CallResult Query(msg::Buffer *msg, int millSecond);
	CallResult Send(char *msg, int size);
	CallResult ReadMsg(msg::Buffer *msg, int millSecond);

private:
	bool Connect();


private:
	bool			m_isConnected;
	net::Socket		m_svr;			//����������
	int				m_nodeId;
	std::string		m_svrIp;
	int				m_svrPort;
	time_t			m_lastConnectFailed;//���һ������ʧ��ʱ��
};

#endif //CLIENT_H