#ifndef MSG_GET_POINT_H
#define MSG_GET_POINT_H

#include "../../Buffer.h"
#include "common/struct/Point.h"

namespace msg
{

class MsgGetPoint : public msg::Buffer
{
public:
	//��������
	int64		m_pointId;
	//��Ӧ����
	Grid::Point	m_point;
	std::vector<std::string> m_getFields;//Ҫȡ���ֶ� null��ʾȡ�����ֶ�

public:
	MsgGetPoint();
	virtual ~MsgGetPoint();

	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_GET_POINT_H