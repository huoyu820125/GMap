#ifndef PROTOCL_BUFFER_H
#define PROTOCL_BUFFER_H

#include "base/Message.h"
#include "Protocl.h"

namespace msg
{

class Buffer : public net::Message
{
public:
	int64					m_connectId;//����Id(TCP�������д)
	unsigned int			m_objectId;//�û�id(TCP�������д)
	unsigned char			m_ip[4];//client ip
	ResultCode::ResultCode	m_code;//�����
	std::string				m_reason;//ԭ��
public:
	Buffer();
	virtual ~Buffer();
	bool Build( bool isResult = false );//���챨�ģ�isResult=true�����Ӧ���ģ�����
	bool Parse();//��������������
	bool FillTransmitParam();//TCP�������ת������

protected:
	unsigned char m_buffer[MAX_MSG_SIZE];
};

}

#endif //PROTOCL_BUFFER_H