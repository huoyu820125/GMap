#ifndef MSG_DELLINEFORPOINT_H
#define MSG_DELLINEFORPOINT_H

#include "../../Buffer.h"
#include "common/Grid.h"

namespace msg
{

//给顶点添加一条边
class MsgDelLineForPoint : public msg::Buffer
{
public:
	//传递数据
	int64	m_pointId;
	int64	m_lineId;
	bool	m_isOut;//边的方向是出去

	//回应数据

public:
	MsgDelLineForPoint();
	virtual ~MsgDelLineForPoint();

	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_DELLINEFORPOINT_H
