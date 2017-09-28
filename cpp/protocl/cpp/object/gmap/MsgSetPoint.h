#ifndef MSG_SET_POINT_H
#define MSG_SET_POINT_H

#include "../../Buffer.h"
#include "common/struct/Point.h"

namespace msg
{

//设置顶点，顶点id<0则创建顶点返回新建顶点id
class MsgSetPoint : public msg::Buffer
{
public:
	//传递数据
	Grid::Point	m_point;

	//回应数据

public:
	MsgSetPoint();
	virtual ~MsgSetPoint();

	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_SET_POINT_H