#ifndef MSG_SETMOVEOPT_H
#define MSG_SETMOVEOPT_H

#include "../../Buffer.h"
#include "common/struct/Line.h"

namespace msg
{

//move查找条件，操作参数
class MsgSetMoveOpt : public msg::Buffer
{
public:
	std::vector<Grid::SELECT_LINE>	m_selectLines;//选择的移动路线
	std::vector<Grid::FILTER>		m_pointFilter;//顶点过滤字段
	bool							m_selectAll;//选取所有字段=true时m_getFields无效
	std::vector<std::string>		m_getFields;//获取的字段
	Grid::Action					m_act; //动作

public:
	MsgSetMoveOpt();
	virtual ~MsgSetMoveOpt();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_SETMOVEOPT_H