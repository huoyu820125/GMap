#ifndef MSG_DEL_LINE_H
#define MSG_DEL_LINE_H

#include "../../Buffer.h"

namespace msg
{

//ɾ����
class MsgDelLine : public msg::Buffer
{
public:
	//��������
	int64 m_lineId;//Ҫɾ���ı�id

	//��Ӧ����
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