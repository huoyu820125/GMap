#ifndef MSG_ADDLINEFORPOINT_H
#define MSG_ADDLINEFORPOINT_H

#include "../../Buffer.h"
#include "common/Grid.h"

namespace msg
{

//给顶点添加一条边
class MsgAddLineForPoint : public msg::Buffer
{
public:
	//传递数据
	int64	m_pointId;
	int64	m_lineId;
	bool	m_isOut;//边的方向是出去
	//回应数据

public:
	MsgAddLineForPoint();
	virtual ~MsgAddLineForPoint();

	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_ADDLINEFORPOINT_H