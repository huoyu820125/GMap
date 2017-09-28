#ifndef MSG_MOVE_MIDS_H
#define MSG_MOVE_MIDS_H

#include "../../Buffer.h"
#include <vector>

namespace msg
{

//�ƶ������е�ǰ�ڵ������ݵĶ���id
class MsgMoveMids : public msg::Buffer
{
public:
	char						m_lineIndex;//m_ids�Ķ����Ǿ����˶��������ƶ�
	bool						m_isLine;//true m_ids�Ǳߣ�false m_ids�Ƕ���
	std::vector<int64>			m_ids;//����id

public:
	MsgMoveMids();
	virtual ~MsgMoveMids();
	bool Build( char lineIndex, bool isLine, std::vector<int64> &ids, int &startPos );
	bool Parse();
};

}
#endif //MSG_MOVE_MIDS_H
