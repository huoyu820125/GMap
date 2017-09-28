#ifndef MSG_SETMOVEOPT_H
#define MSG_SETMOVEOPT_H

#include "../../Buffer.h"
#include "common/struct/Line.h"

namespace msg
{

//move������������������
class MsgSetMoveOpt : public msg::Buffer
{
public:
	std::vector<Grid::SELECT_LINE>	m_selectLines;//ѡ����ƶ�·��
	std::vector<Grid::FILTER>		m_pointFilter;//��������ֶ�
	bool							m_selectAll;//ѡȡ�����ֶ�=trueʱm_getFields��Ч
	std::vector<std::string>		m_getFields;//��ȡ���ֶ�
	Grid::Action					m_act; //����

public:
	MsgSetMoveOpt();
	virtual ~MsgSetMoveOpt();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_SETMOVEOPT_H