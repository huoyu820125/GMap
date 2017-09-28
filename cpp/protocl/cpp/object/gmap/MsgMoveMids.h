#ifndef MSG_MOVE_MIDS_H
#define MSG_MOVE_MIDS_H

#include "../../Buffer.h"
#include <vector>

namespace msg
{

//移动操作中当前节点无数据的对象id
class MsgMoveMids : public msg::Buffer
{
public:
	char						m_lineIndex;//m_ids的对象是经过了多少条边移动
	bool						m_isLine;//true m_ids是边，false m_ids是顶点
	std::vector<int64>			m_ids;//对象id

public:
	MsgMoveMids();
	virtual ~MsgMoveMids();
	bool Build( char lineIndex, bool isLine, std::vector<int64> &ids, int &startPos );
	bool Parse();
};

}
#endif //MSG_MOVE_MIDS_H
