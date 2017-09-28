#ifndef MSG_DEL_LINE_H
#define MSG_DEL_LINE_H

#include "../../Buffer.h"

namespace msg
{

//删除边
class MsgDelLine : public msg::Buffer
{
public:
	//传递数据
	int64 m_lineId;//要删除的边id

	//回应数据
	int64 m_startId;
	int64 m_endId;

public:
	MsgDelLine();
	virtual ~MsgDelLine();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_DEL_LINE_H