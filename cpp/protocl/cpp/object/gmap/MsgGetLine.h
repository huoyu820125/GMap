#ifndef MSG_GET_LINE_H
#define MSG_GET_LINE_H

#include "../../Buffer.h"
#include "common/struct/Line.h"

namespace msg
{

//取得边
class MsgGetLine : public msg::Buffer
{
public:
	//传递数据
	int64		m_lineId;
	//回应数据
	Grid::Line	m_line;
	std::vector<std::string> m_getFields;//要取的字段 null表示取所有字段

public:
	MsgGetLine();
	virtual ~MsgGetLine();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_GET_LINE_H