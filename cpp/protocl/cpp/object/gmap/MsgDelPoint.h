#ifndef MSG_DEL_POINT_H
#define MSG_DEL_POINT_H

#include "../../Buffer.h"

namespace msg
{

//ɾ������
class MsgDelPoint : public msg::Buffer
{
public:
	//��������
	int64 m_pointId;//Ҫɾ���ı�id

	//��Ӧ����

public:
	MsgDelPoint();
	virtual ~MsgDelPoint();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_DEL_POINT_H