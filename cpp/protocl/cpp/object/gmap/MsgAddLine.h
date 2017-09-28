#ifndef MSG_ADD_LINE_H
#define MSG_ADD_LINE_H

#include "../../Buffer.h"
#include "common/struct/Line.h"

namespace msg
{

//连接顶点,返回边id
class MsgAddLine : public msg::Buffer
{
public:
	//传递数据
	Grid::Line m_line;

	//回应数据

public:
	MsgAddLine();
	virtual ~MsgAddLine();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_ADD_LINE_H