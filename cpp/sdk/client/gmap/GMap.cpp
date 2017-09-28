#include "GMap.h"

#include "protocl/cpp/object/gmap/MsgNewId.h"
#include "protocl/cpp/object/gmap/MsgSetPoint.h"
#include "protocl/cpp/object/gmap/MsgGetPoint.h"
#include "protocl/cpp/object/gmap/MsgAddLineForPoint.h"
#include "protocl/cpp/object/gmap/MsgDelLineForPoint.h"
#include "protocl/cpp/object/gmap/MsgDelPoint.h"
#include "protocl/cpp/object/gmap/MsgDelLine.h"
#include "protocl/cpp/object/gmap/MsgAddLine.h"
#include "protocl/cpp/object/gmap/MsgGetLine.h"

#include "protocl/cpp/object/gmap/MsgSetMoveOpt.h"
#include "protocl/cpp/object/gmap/MsgMove.h"

GMap::GMap()
{
}

GMap::~GMap()
{
}

CallResult GMap::NewId(bigint &id)
{
	std::vector<bigint> ids;
	CallResult ret = NewIds(ids, 1);
	if ( ret.isSuccess ) id = ids[0];
	return ret;
}

CallResult GMap::NewIds(std::vector<bigint> &ids, int count)
{
	if ( 0 >= count || Grid::Limit::maxObjectCount < count ) return Call::Error(ResultCode::paramError, "创建id数量必须在1~Grid::Limit::maxObjectCount之间");
	msg::MsgNewId msg;
	msg.m_count = 1;

	CallResult result = Query(&msg, 1);
	if ( !result.isSuccess ) return result;

	int i = 0;
	for ( ; i < count; i++ ) ids.push_back(msg.m_id + i);

	return Call::Success();
}

//取服务配置
CallResult GMap::SetPoint(Grid::Point &point)
{
	msg::MsgSetPoint msg;
	CallResult result;
	msg.m_point.id = point.id;
	msg.m_point.data = point.data;//data中有指针类型数据没有关系，下面构造消息是深拷贝，不会复制指针
	return Query(&msg, 1);
}

CallResult GMap::AddLineForPoint(bigint pointId, bigint lineId, bool isOut)
{
	msg::MsgAddLineForPoint msg;
	msg.m_pointId = pointId;
	msg.m_lineId = lineId;
	msg.m_isOut = isOut;
	return Query(&msg, 1);
}

CallResult GMap::DelLineForPoint(bigint pointId, bigint lineId, bool isOut)
{
	msg::MsgDelLineForPoint msg;
	msg.m_pointId = pointId;
	msg.m_lineId = lineId;
	msg.m_isOut = isOut;
	return Query(&msg, 1);
}

CallResult GMap::GetPoint(bigint pointId, Grid::Point &point)
{
	msg::MsgGetPoint msg;
	msg.m_pointId = pointId;
	CallResult ret = Query(&msg, 1);
	if ( ret.isSuccess ) point = msg.m_point;
	return ret;
}

CallResult GMap::GetLine(bigint lineId, Grid::Line &line)
{
	msg::MsgGetLine msg;
	msg.m_lineId = lineId;
	CallResult ret = Query(&msg, 1);
	if ( ret.isSuccess ) line = msg.m_line;
	return ret;
}

CallResult GMap::DelLine(bigint lineId, bigint &startId, bigint &endId)
{
	msg::MsgDelLine msg;
	msg.m_lineId = lineId;
	CallResult result = Query(&msg, 1);
	if ( !result.isSuccess ) return result;
	startId = msg.m_startId;
	endId = msg.m_endId;

	return Call::Success();
}

CallResult GMap::AddLine(Grid::Line &line)
{
	msg::MsgAddLine msg;

	msg.m_line.id = line.id;
	msg.m_line.startId = line.startId;
	msg.m_line.endId = line.endId;
	msg.m_line.data = line.data;//data中有指针类型数据没有关系，下面构造消息是深拷贝，不会复制指针
	return Query(&msg, 1);
}

CallResult GMap::DelPoint(bigint pointId)
{
	msg::MsgDelPoint msg;
	msg.m_pointId = pointId;
	return Query(&msg, 1);
}

CallResult GMap::SetMoveOpt(std::vector<Grid::SELECT_LINE> &selectLines, std::vector<Grid::FILTER> &pointFilter, 
	bool selectAll, std::vector<std::string> &getFields, Grid::Action &act )
{
	msg::MsgSetMoveOpt msg;
	msg.m_selectLines = selectLines;
	msg.m_pointFilter = pointFilter;
	msg.m_selectAll = selectAll;
	msg.m_getFields = getFields;
	msg.m_act = act;
	if ( !msg.Build() ) return Call::Error(ResultCode::paramError, "报文构造失败");
	return Send(msg, msg.Size());
}

CallResult GMap::Move(bigint searchId, std::vector<bigint> &ids, bool fromPoint, bool filterStartPoint)
{
	msg::MsgMove msg;
	msg.m_fromPoint = fromPoint;
	msg.m_filterStartPoint = filterStartPoint;
	msg.m_ids = ids;
	msg.m_startPos = 0;
	msg.m_searchId = searchId;
	int count = ids.size();

	CallResult ret;
	while ( msg.m_startPos < count )
	{
		if ( !msg.Build() ) return Call::Error(ResultCode::paramError, "报文构造失败");
		ret = Send(msg, msg.Size());
		if ( !ret.isSuccess ) return ret;
	}

	return Call::Success();
}

CallResult GMap::ReadMsg(msg::Buffer *msg, int millSecond)
{
	return Client::ReadMsg(msg, millSecond);
}
