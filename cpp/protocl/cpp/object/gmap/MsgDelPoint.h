#ifndef MSG_DEL_POINT_H
#define MSG_DEL_POINT_H

#include "../../Buffer.h"

namespace msg
{

//删除顶点
class MsgDelPoint : public msg::Buffer
{
public:
	//传递数据
	int64 m_pointId;//要删除的边id

	//回应数据

public:
	MsgDelPoint();
	virtual ~MsgDelPoint();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_DEL_POINT_H