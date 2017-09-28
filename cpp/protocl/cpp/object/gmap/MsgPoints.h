#ifndef MSG_POINTS_H
#define MSG_POINTS_H

#include "../../Buffer.h"
#include <vector>
#include "common/struct/Point.h"

namespace msg
{

//�ƶ�����get/count�������õ����յ�
class MsgPoints : public msg::Buffer
{
public:
	std::vector<Grid::Point>	m_points;//�յ�

public:
	MsgPoints();
	virtual ~MsgPoints();
	bool Build(Grid::Point **points, int count, int &startPos, bool selectAll, std::vector<std::string> &getFields);
	bool Parse();
};

}
#endif //MSG_POINTS_H
