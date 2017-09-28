#include "Worker.h"
#include "mdk/include/mdk/mapi.h"
#include "mdk/include/mdk/atom.h"
#include <cstring>
#include <cstdlib>
#include "client/gsrc/GSrc.h"

#include "protocl/cpp/object/gmap/MsgNewId.h"
#include "protocl/cpp/object/gmap/MsgSetPoint.h"
#include "protocl/cpp/object/gmap/MsgAddLineForPoint.h"
#include "protocl/cpp/object/gmap/MsgDelLineForPoint.h"
#include "protocl/cpp/object/gmap/MsgGetPoint.h"
#include "protocl/cpp/object/gmap/MsgAddLine.h"
#include "protocl/cpp/object/gmap/MsgDelLine.h"
#include "protocl/cpp/object/gmap/MsgGetLine.h"
#include "protocl/cpp/object/gmap/MsgDelPoint.h"
#include "protocl/cpp/object/gmap/MsgSetMoveOpt.h"
#include "protocl/cpp/object/gmap/MsgMove.h"
#include "protocl/cpp/object/gmap/MsgPoints.h"
#include "protocl/cpp/object/gmap/MsgMoveMids.h"

//批量请求
#include "protocl/cpp/object/gmap/MsgSetPoints.h"

#include "common/heap.h"

#ifdef WIN32
#ifdef _DEBUG
#pragma comment ( lib, "mdk_d.lib" )
#else
#pragma comment ( lib, "mdk.lib" )
#endif
#endif

Worker::Worker(std::string cfgname)
{
	char cfgFile[256];
	int pos = cfgname.find_last_of('.');
	if ( -1 == pos )
	{
		printf( "配置文件扩展名错误，应该是类似[xxx.cfg]" );
		mdk::mdk_assert(false);
	}
	std::string en(cfgname.c_str(), pos + 1, cfgname.size() - pos);
	if ( "cfg" != en )
	{
		printf( "配置文件扩展名错误，应该是类似[xxx.cfg]" );
		mdk::mdk_assert(false);
	}

	std::string logName(cfgname.c_str(), 0, pos);
	m_log.SetLogName(logName.c_str());
	m_log.SetPrintLog(true);

	char exeDir[256];
	mdk::GetExeDir( exeDir, 256 );//取得可执行程序位置
	sprintf( cfgFile, "%s/%s", exeDir, cfgname.c_str() );
	m_log.Info("Run", "读取配置：%s", cfgFile);
	if ( !m_cfg.ReadConfig( cfgFile ) )
	{
		m_log.Info( "Error", "配置错误" );
		mdk::mdk_assert(false);
	}
	m_grid.SetWorker(this);

	//获取集群地址
	std::string ip = m_cfg["ClusterConfig"]["ip"];
	int port = m_cfg["ClusterConfig"]["port"];
	GSrc cli;
	cli.SetService(1, ip, port);
	std::map<NetLine::NetLine, std::vector<NODE> > cluster; //集群地址
	CallResult ret = cli.GetServices(cluster, Moudle::gmap);
	if (!ret.isSuccess)
	{
		m_log.Info( "Error", "无法获取集群地址:%s", ret.reason.c_str() );
		mdk::mdk_assert(false);
	}

	//并获取自身节点id
	m_nodeId = -1;
	std::map<NetLine::NetLine, std::vector<NODE> >::iterator it = cluster.begin();
	for ( ; it != cluster.end(); it++ )
	{
		if ( NetLine::localNet != it->first ) continue;//数据库只会在内网
		break;
	}
	ip = (std::string)m_cfg["opt"]["ip"];
	port = m_cfg["opt"]["listen"];
	std::vector<NODE> &nodes = it->second;//集群信息
	int i = 0;
	short nodeId;
	for ( i = 0; i < nodes.size(); i++ )
	{
		nodeId = nodes[i].nodeId;
		if ( ip == nodes[i].ip && port == nodes[i].port )
		{
			m_nodeId = nodes[i].nodeId;
			m_cfg["opt"]["node id"] = m_nodeId;
			m_cfg.Save();
			i++;
			break;
		}
	}
	if ( -1 == m_nodeId )
	{
		m_log.Info( "Error", "节点%s %d未添加到集群配置中", ip.c_str(), port );
		mdk::mdk_assert(false);
	}

	m_log.Info( "Run", "当前节点%d:%s %d", m_nodeId, ip.c_str(), port );
	const char *reason = m_grid.Init(m_nodeId, nodes.size());
	if ( NULL != reason )
	{
		m_log.Info( "Error", "初始化数据库失败:%s", reason );
		mdk::mdk_assert(false);
	}
	m_log.Info( "Run", "监听端口:%d", port );
	OpenNoDelay();
	Listen(port);
}

Worker::~Worker(void)
{
}

mdk::Logger& Worker::Logger()
{
	return m_log;
}

void Worker::OnConnect(mdk::STNetHost &host)
{
	std::string ip;
	int port;
	host.GetAddress(ip, port);
	m_log.Info("Run", "client(%s:%d)连接进来", ip.c_str(), port);

	return;
}

void Worker::OnCloseConnect(mdk::STNetHost &host)
{
	m_moveOpts.erase(host.ID());//删除设置
	m_points.erase(host.ID());//删除批量操作的顶点列表
	std::string ip;
	int port;
	host.GetAddress(ip, port);
	m_log.Info("Run", "client(%s:%d)断开连接", ip.c_str(), port);

	return;
}

void Worker::OnMsg(mdk::STNetHost &host)
{
	msg::Buffer buffer; 
	if ( !host.Recv(buffer, buffer.HeaderSize(), false) ) return;
	if ( !buffer.ReadHeader() )
	{
		if ( !host.IsServer() ) host.Close();
		m_log.Info("Error","报文头错误");
		return;
	}
	if ( !host.Recv(buffer, buffer.Size()) ) return;

	//检查模块类型
	if ( Moudle::gmap != buffer.MoudleId() ) 
	{
		m_log.Info("Error","未预料的模块");
		host.Close();
		return;
	}

	if ( buffer.IsResult() ) //client只能发请求
	{
		m_log.Info("Error","未预料的应答");
		host.Close();
		return;
	}

	//处理请求
	switch ( buffer.Id() )
	{
	case MsgId::newId ://产生一个唯一id
		OnNewId(host, buffer);
		break;
	case MsgId::setPoint ://设置顶点
		OnSetPoint(host, buffer);
		break;
	case MsgId::addLineForPoint ://给顶点添加边
		OnAddLineForPoint(host, buffer);
		break;
	case MsgId::delLineForPoint ://删除顶点的边
		OnDelLineForPoint(host, buffer);
		break;
	case MsgId::getPoint ://取得顶点
		OnGetPoint(host, buffer);
		break;
	case MsgId::addLine ://创建边
		OnAddLine(host, buffer);
		break;
	case MsgId::delLine ://删除边
		OnDelLine(host, buffer);
		break;
	case MsgId::getLine ://取得边
		OnGetLine(host, buffer);
		break;
	case MsgId::delPoint ://删除顶点
		OnDelPoint(host, buffer);
		break;
	case MsgId::setMoveOpt ://设置move
		OnSetMoveOpt(host, buffer);
		break;
	case MsgId::move://多级移动
		OnMove(host, buffer);
		break;
	case MsgId::setPoints://批量设置顶点
		OnSetPoints(host, buffer);
		break;
	default:
		break;
	}

	return;
}

bool Worker::OnNewId(mdk::STNetHost &host, msg::Buffer &buffer)
{
	msg::MsgNewId msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "生成id失败：无法解析报文");
		host.Close();
		return true;
	}

	Grid::Result::Result ret = m_grid.CreateId(msg.m_id, msg.m_count);
	if ( Grid::Result::fileError == ret )
	{
		msg.m_reason = "无法生成id";
		msg.m_code = ResultCode::refuse;
		m_log.Info("Error", "生成id失败：%s", msg.m_reason.c_str() );
	}
	if ( Grid::Result::noIdSource == ret )
	{
		msg.m_reason = "无可用id资源";
		msg.m_code = ResultCode::refuse;
		m_log.Info("Error", "生成id失败：%s", msg.m_reason.c_str() );
	}

	msg.Build(true);
	host.Send(msg, msg.Size());

	return true;
}

bool Worker::OnSetPoint(mdk::STNetHost &host, msg::Buffer &buffer)
{
 	msg::MsgSetPoint msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "设置顶点失败：无法解析报文");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.SetPoint(msg.m_point);
	msg.m_point.Release();//释放内存
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "设置顶点失败：%s", msg.m_reason.c_str() );
	}
	msg.Build(true);
	host.Send(msg, msg.Size());

	return true;
}

bool Worker::OnAddLineForPoint(mdk::STNetHost &host, msg::Buffer &buffer)
{
	msg::MsgAddLineForPoint msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "给顶点添加边失败：无法解析报文");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.AddLineForPoint(msg.m_pointId, msg.m_lineId, msg.m_isOut);
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "给顶点添加边失败：%s", msg.m_reason.c_str() );
	}
	msg.Build(true);
	host.Send(msg, msg.Size());

	return true;
}

bool Worker::OnDelLineForPoint(mdk::STNetHost &host, msg::Buffer &buffer)
{
	msg::MsgDelLineForPoint msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "删除顶点的边失败：无法解析报文");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.DelLineForPoint(msg.m_pointId, msg.m_lineId, msg.m_isOut);
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "删除顶点的边失败：%s", msg.m_reason.c_str() );
	}
	msg.Build(true);
	host.Send(msg, msg.Size());

	return true;
}

bool Worker::OnGetPoint(mdk::STNetHost &host, msg::Buffer &buffer)
{
	msg::MsgGetPoint msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "取顶点失败：无法解析报文");
		host.Close();
		return true;
	}

	Grid::Point *pPoint = NULL;
	CallResult ret = m_grid.GetPoint(pPoint, msg.m_pointId);
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
	}
	else msg.m_point = *pPoint;

	msg.Build(true);
	host.Send(msg, msg.Size());
	return true;
}

bool Worker::OnAddLine(mdk::STNetHost &host, msg::Buffer &buffer)
{
	msg::MsgAddLine msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "连接顶点失败：无法解析报文");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.AddLine(msg.m_line);
	msg.m_line.Release();//释放内存
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "连接顶点失败：%s", msg.m_reason.c_str() );
	}
	msg.Build(true);
	host.Send(msg, msg.Size());

	return true;
}

bool Worker::OnDelLine(mdk::STNetHost &host, msg::Buffer &buffer)
{
	msg::MsgDelLine msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "删除边失败：无法解析报文");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.DelLine(msg.m_lineId, msg.m_startId, msg.m_endId);
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "删除边失败：%s", msg.m_reason.c_str() );
	}
	msg.Build(true);
	host.Send(msg, msg.Size());

	return true;
}

bool Worker::OnGetLine(mdk::STNetHost &host, msg::Buffer &buffer)
{
	msg::MsgGetLine msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "查询边失败：无法解析报文");
		host.Close();
		return true;
	}

	Grid::Line *pLine;
	CallResult ret = m_grid.GetLine(pLine, msg.m_lineId);
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
	}
	else msg.m_line = *pLine;

	msg.Build(true);
	host.Send(msg, msg.Size());

	return true;
}

bool Worker::OnDelPoint(mdk::STNetHost &host, msg::Buffer &buffer)
{
	msg::MsgDelPoint msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "删除顶点失败：无法解析报文");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.DelPoint(msg.m_pointId);
	if ( !ret.isSuccess )
	{
		msg.m_code = (ResultCode::ResultCode)ret.code;
		msg.m_reason = ret.reason;
	}
	msg.Build(true);
	host.Send(msg, msg.Size());

	return true;
}

bool Worker::OnSetMoveOpt(mdk::STNetHost &host, msg::Buffer &buffer)
{
	msg::MsgSetMoveOpt msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "设置查询失败：无法解析报文");
		host.Close();
		return true;
	}
	MOVE_OPT &opt = m_moveOpts[host.ID()];
	opt.ids.clear();
	opt.selectLines = msg.m_selectLines;
	opt.pointFilter = msg.m_pointFilter;
	int i = 0;
	int j = 0;
	for ( ; i < opt.selectLines.size(); i++ )
	{
		for ( j = 0; j < opt.selectLines[i].selectField.size(); j++ ) 
		{
			opt.selectLines[i].selectField[j].hashValue 
				= mdk::Map::hash( 
				(char*)(opt.selectLines[i].selectField[j].fieldName.c_str()), 
				opt.selectLines[i].selectField[j].fieldName.size() );
		}
	}
	for ( i = 0; i < opt.pointFilter.size(); i++ )
	{
		opt.pointFilter[i].hashValue 
			= mdk::Map::hash( 
			(char*)(opt.pointFilter[i].fieldName.c_str()), 
			opt.pointFilter[i].fieldName.size() );
	}
	opt.selectAll = msg.m_selectAll;
	opt.getFields = msg.m_getFields;
	opt.act = msg.m_act;

	return true;
}

bool Worker::OnMove( mdk::STNetHost &host, msg::Buffer &buffer )
{
//	mdk::uint64 startMove = mdk::MillTime();
	msg::MsgMove msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "查询失败：无法解析报文");
		host.Close();
		return true;
	}

	std::map<int, MOVE_OPT>::iterator it = m_moveOpts.find(host.ID());
	if ( it == m_moveOpts.end() ) 
	{
		msg.m_reason = "没有初始化查询设置";
		msg.m_code = ResultCode::refuse;
		m_log.Info("Error", "拒绝查询：%s", msg.m_reason.c_str() );
		msg.Build( true );
		host.Send(msg, msg.Size());
		return true;
	}

	MOVE_OPT &opt = it->second;//拿到查询设置,保存参数
	int i = 0;
	int idsSize = msg.m_ids.size();
	for ( i = 0; i < idsSize; i++ ) opt.ids.push_back(msg.m_ids[i]);

	if ( !msg.m_moveStart ) return true;//参数未接收完成，等待数据

	if ( 0 == opt.selectLines.size() )
	{
		if (Grid::unlink == opt.act)
		{
			msg.m_reason = "unlink操作必须移动";
			msg.m_code = ResultCode::refuse;
			m_log.Info("Error", "拒绝查询：%s", msg.m_reason.c_str() );
			msg.Build( true );
			host.Send(msg, msg.Size());
			return true;
		}
		if (msg.m_fromPoint && !msg.m_filterStartPoint 
			&& Grid::count == opt.act )
		{
			msg.m_reason = "count操作必须移动 or 设置过滤条件";
			msg.m_code = ResultCode::refuse;
			m_log.Info("Error", "拒绝查询：%s", msg.m_reason.c_str() );
			msg.Build( true );
			host.Send(msg, msg.Size());
			return true;
		}
	}
	if ( Grid::del == opt.act || Grid::unlink == opt.act )
	{
		if ( 1 < opt.selectLines.size() )
		{
			msg.m_reason = "unlink与del操作不支持多级移动";
			msg.m_code = ResultCode::refuse;
			m_log.Info("Error", "拒绝查询：%s", msg.m_reason.c_str() );
			msg.Build( true );
			host.Send(msg, msg.Size());
			return true;
		}
	}

	msg::MsgPoints replyData;
	std::vector<Grid::REMOTE_DATA> movesParam;

	m_endPoints.clear();
	CallResult ret = m_grid.Move(msg.m_searchId, m_endPoints, movesParam, 
		msg.m_fromPoint, msg.m_filterStartPoint, opt.ids, 
		opt.selectLines, opt.pointFilter, opt.act );
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "查询失败：%s", msg.m_reason.c_str() );
		msg.Build(true);
		host.Send(msg, msg.Size());
		return true;
	}

	//发送终点
	int startPos = 0;
	int sendCount = 0;
	while ( startPos < m_endPoints.size )
	{
		replyData.Build(m_endPoints.datas, m_endPoints.size, startPos, opt.selectAll, opt.getFields);
		host.Send(replyData, replyData.Size());
		sendCount++;
	}

	//发送需要到其它节点查询的边与中间顶点
	msg::MsgMoveMids mid;
	int listSize;
	for ( i = 0; i < movesParam.size(); i++ )
	{
		listSize = movesParam[i].lineIds.size();
		for ( startPos = 0; startPos < listSize; )
		{
			mid.Build(movesParam[i].lineIndex, true, movesParam[i].lineIds, startPos);
			host.Send(mid, mid.Size());
			sendCount++;
		}

		listSize = movesParam[i].pointIds.size();
		for ( startPos = 0; startPos < listSize; )
		{
			mid.Build(movesParam[i].lineIndex, false, movesParam[i].pointIds, startPos);
			host.Send(mid, mid.Size());
			sendCount++;
		}
	}

	//发送成功通知
	msg.m_code = ResultCode::success;
	msg.Build(true);
	host.Send(msg, msg.Size());
	sendCount++;
// 	startMove = mdk::MillTime() - startMove;
// 	m_log.Info("Debug", "查询用时：%lld毫秒", startMove);
	 
	return true;
}

bool Worker::OnSetPoints( mdk::STNetHost &host, msg::Buffer &buffer )
{
	msg::MsgSetPoints msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "批量设置顶点失败：无法解析报文");
		host.Close();
		return true;
	}
	std::vector<Grid::Point> &points = m_points[host.ID()];//删除批量操作的顶点列表
	int i = 0;
	for ( ; i < msg.m_points.size(); i++ )
	{
		points.push_back(msg.m_points[i]);
	}
	if ( !msg.m_isEnd )//列表未完成，等待数据
	{
		return true;
	}

	if ( msg.m_isCreate )//新建顶点批量写入到文件末尾
	{
		CallResult ret = m_grid.CreatePoints(points);
		if ( !ret.isSuccess )
		{
			msg.m_code = (ResultCode::ResultCode)ret.code;
			msg.m_reason = ret.reason;
		}
	}
	else//编辑顶点，不能批量操作
	{
		CallResult ret;
		for ( i = 0; i < points.size(); i++ )
		{
			ret = m_grid.SetPoint(points[i]);
			if ( !ret.isSuccess )
			{
				msg.m_code = (ResultCode::ResultCode)ret.code;
				msg.m_reason = ret.reason;
				break;
			}
		}
	}
	msg.Build(true);
	host.Send(msg, msg.Size());
	points.clear();

	return true;
}
