#ifndef MSG_GET_LINE_H
#define MSG_GET_LINE_H

#include "../../Buffer.h"
#include "common/struct/Line.h"

namespace msg
{

//ȡ�ñ�
class MsgGetLine : public msg::Buffer
{
public:
	//��������
	int64		m_lineId;
	//��Ӧ����
	Grid::Line	m_line;
	std::vector<std::string> m_getFields;//Ҫȡ���ֶ� null��ʾȡ�����ֶ�

public:
	MsgGetLine();
	virtual ~MsgGetLine();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_GET_LINE_H