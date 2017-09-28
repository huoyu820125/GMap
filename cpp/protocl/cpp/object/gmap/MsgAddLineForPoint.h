#ifndef MSG_ADDLINEFORPOINT_H
#define MSG_ADDLINEFORPOINT_H

#include "../../Buffer.h"
#include "common/Grid.h"

namespace msg
{

//���������һ����
class MsgAddLineForPoint : public msg::Buffer
{
public:
	//��������
	int64	m_pointId;
	int64	m_lineId;
	bool	m_isOut;//�ߵķ����ǳ�ȥ
	//��Ӧ����

public:
	MsgAddLineForPoint();
	virtual ~MsgAddLineForPoint();

	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_ADDLINEFORPOINT_H