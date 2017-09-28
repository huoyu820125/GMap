#ifndef MSG_GET_POINT_H
#define MSG_GET_POINT_H

#include "../../Buffer.h"
#include "common/struct/Point.h"

namespace msg
{

class MsgGetPoint : public msg::Buffer
{
public:
	//传递数据
	int64		m_pointId;
	//回应数据
	Grid::Point	m_point;
	std::vector<std::string> m_getFields;//要取的字段 null表示取所有字段

public:
	MsgGetPoint();
	virtual ~MsgGetPoint();

	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_GET_POINT_H