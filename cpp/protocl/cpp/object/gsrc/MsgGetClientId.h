#ifndef MSG_GET_CLIENT_ID_H
#define MSG_GET_CLIENT_ID_H

#include "../../Buffer.h"

namespace msg
{

//产生一个唯一client Id
class MsgGetClientId : public msg::Buffer
{
public:
	//传递数据

	//回应数据
	int64		m_clientId;//cient唯一id

public:
	MsgGetClientId();
	virtual ~MsgGetClientId();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_GET_CLIENT_ID_H
