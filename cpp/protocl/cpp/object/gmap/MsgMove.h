#ifndef MSG_MOVEN_H
#define MSG_MOVEN_H

#include "../../Buffer.h"
#include <vector>

namespace msg
{

//从起顶点/边开始移动N次，并执行预设的行为
class MsgMove : public msg::Buffer
{
public:
	//传递数据
	int64						m_searchId;//搜索唯一id
	bool						m_fromPoint;//从顶点开始移动
	bool						m_filterStartPoint;//过滤起点
	std::vector<int64>			m_ids;//起点id列表
	bool						m_moveStart;//开始移动
	int							m_startPos;//id列表打包开始位置
	//回应数据

public:
	MsgMove();
	virtual ~MsgMove();
	 
 	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_MOVEN_H