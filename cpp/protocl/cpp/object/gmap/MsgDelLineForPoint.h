#ifndef MSG_DELLINEFORPOINT_H
#define MSG_DELLINEFORPOINT_H

#include "../../Buffer.h"
#include "common/Grid.h"

namespace msg
{

//���������һ����
class MsgDelLineForPoint : public msg::Buffer
{
public:
	//��������
	int64	m_pointId;
	int64	m_lineId;
	bool	m_isOut;//�ߵķ����ǳ�ȥ

	//��Ӧ����

public:
	MsgDelLineForPoint();
	virtual ~MsgDelLineForPoint();

	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_DELLINEFORPOINT_H
