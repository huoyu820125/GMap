#ifndef MSG_ADD_LINE_H
#define MSG_ADD_LINE_H

#include "../../Buffer.h"
#include "common/struct/Line.h"

namespace msg
{

//���Ӷ���,���ر�id
class MsgAddLine : public msg::Buffer
{
public:
	//��������
	Grid::Line m_line;

	//��Ӧ����

public:
	MsgAddLine();
	virtual ~MsgAddLine();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_ADD_LINE_H