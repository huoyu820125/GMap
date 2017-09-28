#ifndef MSG_SET_POINT_H
#define MSG_SET_POINT_H

#include "../../Buffer.h"
#include "common/struct/Point.h"

namespace msg
{

//���ö��㣬����id<0�򴴽����㷵���½�����id
class MsgSetPoint : public msg::Buffer
{
public:
	//��������
	Grid::Point	m_point;

	//��Ӧ����

public:
	MsgSetPoint();
	virtual ~MsgSetPoint();

	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_SET_POINT_H