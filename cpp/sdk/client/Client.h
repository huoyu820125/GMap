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
		检查服务是否就绪
		对于未就绪的服务，5分种内不再检查服务状态，直接认为是服务未就绪
	*/
	bool IsReady();
	bool IsEnd();//数据接收完成
	void Close();
protected:
	CallResult Query(msg::Buffer *msg, int millSecond);
	CallResult Send(char *msg, int size);
	CallResult ReadMsg(msg::Buffer *msg, int millSecond);

private:
	bool Connect();


private:
	bool			m_isConnected;
	net::Socket		m_svr;			//服务器连接
	int				m_nodeId;
	std::string		m_svrIp;
	int				m_svrPort;
	time_t			m_lastConnectFailed;//最后一次连接失败时间
};

#endif //CLIENT_H