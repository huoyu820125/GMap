#ifndef PROTOCL_BUFFER_H
#define PROTOCL_BUFFER_H

#include "base/Message.h"
#include "Protocl.h"

namespace msg
{

class Buffer : public net::Message
{
public:
	int64					m_connectId;//连接Id(TCP接入服填写)
	unsigned int			m_objectId;//用户id(TCP接入服填写)
	unsigned char			m_ip[4];//client ip
	ResultCode::ResultCode	m_code;//结果码
	std::string				m_reason;//原因
public:
	Buffer();
	virtual ~Buffer();
	bool Build( bool isResult = false );//构造报文，isResult=true构造回应报文，发送
	bool Parse();//解析解析，接收
	bool FillTransmitParam();//TCP服务填充转发参数

protected:
	unsigned char m_buffer[MAX_MSG_SIZE];
};

}

#endif //PROTOCL_BUFFER_H