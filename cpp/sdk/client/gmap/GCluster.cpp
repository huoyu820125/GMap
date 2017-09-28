#include "GCluster.h"
#include "protocl/cpp/object/gmap/MsgMove.h"
#include "protocl/cpp/object/gmap/MsgPoints.h"
#include "protocl/cpp/object/gmap/MsgMoveMids.h"

GCluster::GCluster()
{
	m_clientId = -1;
	m_searchIndex = 0x3FFFFF;
	m_gsrc.SetService(1, "192.168.2.225", 8888);
}

GCluster::~GCluster()
{
}

bool GCluster::AddNode(NODE &node)
{
	if ( m_nodes.end() != m_nodes.find(node.nodeId) ) return false;
	m_nodes[node.nodeId].SetService(node.nodeId, node.ip, node.port);

	m_itNextNode = m_nodes.begin();
	return true;
}

bool GCluster::AddNodes(std::vector<NODE> &nodes)
{
	int i = 0;
	for ( i = 0; i < nodes.size(); i++ )
	{
		AddNode(nodes[i]);
	}

	return true;
}

GMap& GCluster::NextNode()
{
	int i = 0;
	std::map<int, GMap>::iterator itSelect = m_itNextNode;
	for ( i = 0; i < m_nodes.size(); i++ )
	{
		if ( m_itNextNode->second.IsReady() ) break;

		m_itNextNode++;
		if ( m_itNextNode == m_nodes.end() ) m_itNextNode = m_nodes.begin();
		itSelect = m_itNextNode;
	}
	m_itNextNode++;
	if ( m_itNextNode == m_nodes.end() ) m_itNextNode = m_nodes.begin();

	return itSelect->second;
}

GMap& GCluster::FindNode(bigint dataId)
{
	int nodeId = Grid::FindNodeId(dataId, m_nodes.size());
	return m_nodes[nodeId];
}

CallResult GCluster::InitClientId()
{
	CallResult ret = m_gsrc.GetClientId(m_clientId);
	if ( !ret.isSuccess ) return ret;
	m_searchId = m_clientId;
	m_searchIndex = 0;

	return ret;
}

CallResult GCluster::NewId(bigint &id)
{
	return NextNode().NewId(id);
}

void GCluster::CloseCluster()
{
	std::map<int, GMap>::iterator it = m_nodes.begin();
	for ( ; it != m_nodes.end(); it++ )
	{
		it->second.Close();
	}
}

CallResult GCluster::SetPoint(Grid::Point &point)
{
	if ( point.data.size() > Grid::Limit::maxFieldCount ) return Call::Error(ResultCode::paramError, "太多字段");//属性最多64个

	if ( 0 >= point.id ) 
	{
		CallResult ret = NewId(point.id);
		if ( !ret.isSuccess ) return ret;
	}
	return FindNode(point.id).SetPoint(point);
}

CallResult GCluster::AddLineForPoint(bigint pointId, bigint lineId, bool isOut)
{
	if ( pointId <= 0 ) return Call::Error(ResultCode::paramError, "顶点Id<=0");
	if ( lineId <= 0 ) return Call::Error(ResultCode::paramError, "边Id<=0");

	return FindNode(pointId).AddLineForPoint(pointId, lineId, isOut);
}

CallResult GCluster::DelLineForPoint(bigint pointId, bigint lineId, bool isOut)
{
	if ( pointId <= 0 ) return Call::Error(ResultCode::paramError, "顶点Id<=0");
	if ( lineId <= 0 ) return Call::Error(ResultCode::paramError, "边Id<=0");

	return FindNode(pointId).DelLineForPoint(pointId, lineId, isOut);
}

CallResult GCluster::GetPoint(bigint pointId, Grid::Point &point)
{
	if ( pointId <= 0 ) return Call::Error(ResultCode::paramError, "顶点Id<=0");

	return FindNode(pointId).GetPoint(pointId, point);
}

CallResult GCluster::GetLine(bigint lineId, Grid::Line &line)
{
	if ( lineId <= 0 ) return Call::Error(ResultCode::paramError, "边Id<=0");

	return FindNode(lineId).GetLine(lineId, line);
}

CallResult GCluster::DelLine(bigint lineId, bigint &startId, bigint &endId)
{
	if ( lineId <= 0 ) return Call::Error(ResultCode::paramError, "边Id<=0");

	return FindNode(lineId).DelLine(lineId, startId, endId);
}

CallResult GCluster::Link(Grid::Line &line)
{
	if ( line.startId <= 0 ) return Call::Error(ResultCode::paramError, "起点Id<=0");
	if ( line.endId <= 0 ) return Call::Error(ResultCode::paramError, "终点Id<=0");
	if ( line.data.size() > Grid::Limit::maxFieldCount ) return Call::Error(ResultCode::paramError, "太多字段");//属性最多64个

	if ( 0 >= line.id ) 
	{
		CallResult ret = NewId(line.id);
		if ( !ret.isSuccess ) return ret;
	}
	CallResult ret = AddLineForPoint(line.endId, line.id, false);
	if ( !ret.isSuccess ) return ret;
	ret = AddLineForPoint(line.startId, line.id, true);
	if ( !ret.isSuccess ) 
	{
		DelLineForPoint(line.endId, line.id, false);
		return ret;
	}

	ret = FindNode(line.id).AddLine(line);
	if ( !ret.isSuccess )
	{
		DelLineForPoint(line.startId, line.id, true);
		DelLineForPoint(line.endId, line.id, false);
	}
	return ret;
}

CallResult GCluster::DelPoint(bigint pointId)
{
	if ( pointId <= 0 ) return Call::Error(ResultCode::paramError, "顶点Id<=0");

	return FindNode(pointId).DelPoint(pointId);
}

CallResult GCluster::Unlink(bigint lineId)
{
	if ( lineId <= 0 ) return Call::Error(ResultCode::paramError, "边Id<=0");
	bigint startId, endId;
	return FindNode(lineId).DelLine(lineId, startId, endId);
}

int g_repeatCount = 0;
CallResult GCluster::Move(std::vector<Grid::Point> &points, 
	std::vector<bigint> &ids,
	std::vector<Grid::SELECT_LINE> &selectLines, std::vector<Grid::FILTER> &pointFilter, 
	bool selectAll, std::vector<std::string> &getFields, Grid::Action act)
{
	//生成新的分布式唯一搜索id
	if ( m_searchIndex >= 0x3FFFFF )//请求新id
	{
		CallResult ret = InitClientId();
		if ( !ret.isSuccess ) return ret;
	}
	else//使用新检索id
	{
		m_searchIndex++;
		m_searchId++;
	}

	bool startFilter = false;
	if ( 0 == selectLines.size() ) startFilter = true;

	//开始请求查询
	g_repeatCount = 0;
	std::map<int, Grid::REMOTE_DATA> movesParam;
	std::map<bigint, bool> repeat;
	CallResult ret = Move(points, repeat, 
		ids, true, startFilter, movesParam,
		selectLines, pointFilter, 
		selectAll, getFields, act);
	if ( !ret.isSuccess ) 
	{
		CloseCluster();
		return ret;
	}


	int i = 0;
	int startLine = 0;
	while ( 0 < selectLines.size() )
	{
		//删除移动完成的路线
		startLine = 0;
		for ( i = 0; i < Grid::Limit::maxMoveCount && 0 < selectLines.size(); i++ )
		{
			Grid::REMOTE_DATA &moveParam = movesParam[i];
			if ( 0 == moveParam.lineIds.size() && 0 == moveParam.pointIds.size() ) 
			{
				selectLines.erase(selectLines.begin());
				continue;
			}
			startLine = i;
			break;
		}
		if ( 0 == selectLines.size() ) break;

		//未移动完成的路线重新编号
		std::map<int, Grid::REMOTE_DATA> movesParam2;
		for ( ; i < Grid::Limit::maxMoveCount; i++ )
		{
			Grid::REMOTE_DATA &moveParam = movesParam[i];
			moveParam.lineIndex -= startLine;
			movesParam2[moveParam.lineIndex] = moveParam;
		}
		//处理第一条路线
		Grid::REMOTE_DATA &moveParam = movesParam2[0];
		if ( 0 != moveParam.lineIds.size() )
		{
			ret = Move(points, repeat, moveParam.lineIds, false, true, movesParam2, selectLines, pointFilter, selectAll, getFields, act);
			if ( !ret.isSuccess ) 
			{
				CloseCluster();
				return ret;
			}
			moveParam.lineIds.clear();
		}
		//删除当前路线
		movesParam.clear();
		for ( i = 1; i < selectLines.size(); i++ )
		{
			Grid::REMOTE_DATA &moveParam = movesParam2[i];
			moveParam.lineIndex -= 1;
			movesParam[moveParam.lineIndex] = moveParam;
		}
		selectLines.erase(selectLines.begin());
		//处理当前路线终点
		if ( 0 != moveParam.pointIds.size() )
		{
			ret = Move(points, repeat, moveParam.pointIds, true, true, movesParam, selectLines, pointFilter, selectAll, getFields, act);
			if ( !ret.isSuccess )
			{
				CloseCluster();
				return ret;
			}
			continue;
		}
	}

	return Call::Success();
}

CallResult GCluster::Move(std::vector<Grid::Point> &points, std::map<bigint, bool> &repeat, 
	std::vector<bigint> &ids, bool fromPoint, bool filterStartPoint, std::map<int, Grid::REMOTE_DATA> &movesParam,
	std::vector<Grid::SELECT_LINE> &selectLines, std::vector<Grid::FILTER> &pointFilter, 
	bool selectAll, std::vector<std::string> &getFields, Grid::Action act)
{
	int nodeCount = m_nodes.size();
	int i = 0;
	int nodeId;
	typedef struct CALL_SERVER
	{
		std::vector<bigint> ids;
		int callCount;
	}CALL_SERVER;
	CALL_SERVER querys[1001];
	std::map<int, std::vector<bigint> > clusterIds;//按照节点分组
	std::map<bigint, bool> repeatQuery;
	int repeatCount = 0;
	for ( i = 0; i < ids.size(); i++ )
	{
		if ( repeatQuery.end() != repeatQuery.find(ids[i]) ) 
		{
			repeatCount++;
			continue;//过滤重复id
		}
		repeatQuery.insert(std::map<bigint, bool>::value_type(ids[i], true));
		nodeId = Grid::FindNodeId(ids[i], nodeCount);
		querys[nodeId].ids.push_back(ids[i]);
	}
	ids.clear();

	//分布式计算从起点查询
	CallResult ret;
	for ( nodeId = 1; nodeId < 1001; nodeId++ )
	{
		if ( 0 == querys[nodeId].ids.size() ) continue;

		GMap &client = m_nodes[nodeId];
		querys[nodeId].callCount = 0;
		ret = client.SetMoveOpt(selectLines, pointFilter, selectAll, getFields, act);
		if ( !ret.isSuccess ) return ret;
		ret = client.Move(m_searchId, querys[nodeId].ids, fromPoint, filterStartPoint);
		if ( !ret.isSuccess ) return ret;

		querys[nodeId].callCount = querys[nodeId].ids.size() / 1000;
		if ( 0 != querys[nodeId].ids.size() % 1000 ) querys[nodeId].callCount += 1;
	}

	//合并接收结果
	msg::Buffer buf;

	int recvCount = 0;
	for ( nodeId = 0; nodeId < 1001; nodeId++ )
	{
		if ( 0 == querys[nodeId].ids.size() ) continue;

		GMap &client = m_nodes[nodeId];
		
		
		while ( true )
		{
			ret = client.ReadMsg(&buf, 1);
			if ( !ret.isSuccess ) return ret;
			if ( MsgId::move != buf.Id() && MsgId::points != buf.Id() && MsgId::moveMids != buf.Id() )
			{
				client.Close();
				return Call::Error(ResultCode::msgError, "服务端非法报文");
			}
			//处理数据
			recvCount++;
			if ( MsgId::move == buf.Id() )
			{
				querys[nodeId].callCount--;
				msg::MsgMove msg;
				memcpy( msg, buf, buf.Size() );
				if ( !msg.Parse() ) 
				{
					client.Close();
					return Call::Error(ResultCode::msgError, "服务端报文格式异常");
				}
				if ( ResultCode::success != msg.m_code ) 
				{
					return Call::Error(msg.m_code, msg.m_reason);
				}
				if ( 0 == querys[nodeId].callCount ) 
				{
					break;//请求成功
				}
				continue;
			}
			if ( MsgId::points == buf.Id() )
			{
				msg::MsgPoints msg;
				memcpy( msg, buf, buf.Size() );
				if ( !msg.Parse() ) 
				{
					m_nodes[nodeId].Close();
					//释放解析到一半的顶点内存
					for ( i = 0; i < msg.m_points.size(); i++ ) msg.m_points[i].Release();
					return Call::Error(ResultCode::msgError, "服务端报文格式异常");
				}
				int count = msg.m_points.size();
				std::map<bigint, bool>::iterator it;
				for ( i = 0; i < count; i++ )
				{
					if ( repeat.end() != repeat.find(msg.m_points[i].id) )//终点去重复
					{
						msg.m_points[i].Release();
						g_repeatCount++;
						continue;
					}
					repeat.insert(std::map<bigint, bool>::value_type(msg.m_points[i].id, true));
					points.push_back(msg.m_points[i]);
				}
			}
			else //MsgId::moveMids
			{
				msg::MsgMoveMids msg;
				memcpy( msg, buf, buf.Size() );
				if ( !msg.Parse() ) 
				{
					client.Close();
					return Call::Error(ResultCode::msgError, "服务端报文格式异常");
				}
				int count = msg.m_ids.size();
				Grid::REMOTE_DATA &moveParam = movesParam[msg.m_lineIndex];
				moveParam.lineIndex = msg.m_lineIndex;
				if ( msg.m_isLine )
				{
					for ( i = 0; i < count; i++ ) moveParam.lineIds.push_back(msg.m_ids[i]);
					continue;
				}
				std::map<bigint, bool>::iterator it;
				for ( i = 0; i < count; i++ )
				{
					moveParam.pointIds.push_back(msg.m_ids[i]);
				}
			}
		}
	}

	return Call::Success();
}
