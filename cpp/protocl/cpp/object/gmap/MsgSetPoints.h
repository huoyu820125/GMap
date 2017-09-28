#ifndef MSG_SET_POINTS_H
#define MSG_SET_POINTS_H

#include "../../Buffer.h"
#include "common/struct/Point.h"

namespace msg
{

//批量设置顶点
class MsgSetPoints : public msg::Buffer
{
public:
	//请求参数
	bool							m_isEnd;	//顶点列表结束
	bool							m_isCreate;	//是创建操作
	std::vector<Grid::Point>		m_points;	//顶点列表
	int								m_startPos;	//从第几个顶点开始打包
	//回应数据

public:
	MsgSetPoints();
	virtual ~MsgSetPoints();


	bool Build( bool isResult = false );
	bool Parse();
};


}
#endif //MSG_SET_POINTS_H