#ifndef MSG_NEW_ID_H
#define MSG_NEW_ID_H

#include "../../Buffer.h"

namespace msg
{

//产生m_count个唯一Id
class MsgNewId : public msg::Buffer
{
public:
	//传递数据
	int  m_count;//产生id数量

	//回应数据
	int64 m_id;//唯一id

public:
	MsgNewId();
	virtual ~MsgNewId();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_NEW_ID_H