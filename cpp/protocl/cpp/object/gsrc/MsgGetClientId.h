#ifndef MSG_GET_CLIENT_ID_H
#define MSG_GET_CLIENT_ID_H

#include "../../Buffer.h"

namespace msg
{

//����һ��Ψһclient Id
class MsgGetClientId : public msg::Buffer
{
public:
	//��������

	//��Ӧ����
	int64		m_clientId;//cientΨһid

public:
	MsgGetClientId();
	virtual ~MsgGetClientId();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_GET_CLIENT_ID_H
