#ifndef MSG_SET_POINTS_H
#define MSG_SET_POINTS_H

#include "../../Buffer.h"
#include "common/struct/Point.h"

namespace msg
{

//�������ö���
class MsgSetPoints : public msg::Buffer
{
public:
	//�������
	bool							m_isEnd;	//�����б����
	bool							m_isCreate;	//�Ǵ�������
	std::vector<Grid::Point>		m_points;	//�����б�
	int								m_startPos;	//�ӵڼ������㿪ʼ���
	//��Ӧ����

public:
	MsgSetPoints();
	virtual ~MsgSetPoints();


	bool Build( bool isResult = false );
	bool Parse();
};


}
#endif //MSG_SET_POINTS_H