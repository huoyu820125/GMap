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

//��������
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
		printf( "�����ļ���չ������Ӧ��������[xxx.cfg]" );
		mdk::mdk_assert(false);
	}
	std::string en(cfgname.c_str(), pos + 1, cfgname.size() - pos);
	if ( "cfg" != en )
	{
		printf( "�����ļ���չ������Ӧ��������[xxx.cfg]" );
		mdk::mdk_assert(false);
	}

	std::string logName(cfgname.c_str(), 0, pos);
	m_log.SetLogName(logName.c_str());
	m_log.SetPrintLog(true);

	char exeDir[256];
	mdk::GetExeDir( exeDir, 256 );//ȡ�ÿ�ִ�г���λ��
	sprintf( cfgFile, "%s/%s", exeDir, cfgname.c_str() );
	m_log.Info("Run", "��ȡ���ã�%s", cfgFile);
	if ( !m_cfg.ReadConfig( cfgFile ) )
	{
		m_log.Info( "Error", "���ô���" );
		mdk::mdk_assert(false);
	}
	m_grid.SetWorker(this);

	//��ȡ��Ⱥ��ַ
	std::string ip = m_cfg["ClusterConfig"]["ip"];
	int port = m_cfg["ClusterConfig"]["port"];
	GSrc cli;
	cli.SetService(1, ip, port);
	std::map<NetLine::NetLine, std::vector<NODE> > cluster; //��Ⱥ��ַ
	CallResult ret = cli.GetServices(cluster, Moudle::gmap);
	if (!ret.isSuccess)
	{
		m_log.Info( "Error", "�޷���ȡ��Ⱥ��ַ:%s", ret.reason.c_str() );
		mdk::mdk_assert(false);
	}

	//����ȡ����ڵ�id
	m_nodeId = -1;
	std::map<NetLine::NetLine, std::vector<NODE> >::iterator it = cluster.begin();
	for ( ; it != cluster.end(); it++ )
	{
		if ( NetLine::localNet != it->first ) continue;//���ݿ�ֻ��������
		break;
	}
	ip = (std::string)m_cfg["opt"]["ip"];
	port = m_cfg["opt"]["listen"];
	std::vector<NODE> &nodes = it->second;//��Ⱥ��Ϣ
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
		m_log.Info( "Error", "�ڵ�%s %dδ��ӵ���Ⱥ������", ip.c_str(), port );
		mdk::mdk_assert(false);
	}

	m_log.Info( "Run", "��ǰ�ڵ�%d:%s %d", m_nodeId, ip.c_str(), port );
	const char *reason = m_grid.Init(m_nodeId, nodes.size());
	if ( NULL != reason )
	{
		m_log.Info( "Error", "��ʼ�����ݿ�ʧ��:%s", reason );
		mdk::mdk_assert(false);
	}
	m_log.Info( "Run", "�����˿�:%d", port );
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
	m_log.Info("Run", "client(%s:%d)���ӽ���", ip.c_str(), port);

	return;
}

void Worker::OnCloseConnect(mdk::STNetHost &host)
{
	m_moveOpts.erase(host.ID());//ɾ������
	m_points.erase(host.ID());//ɾ�����������Ķ����б�
	std::string ip;
	int port;
	host.GetAddress(ip, port);
	m_log.Info("Run", "client(%s:%d)�Ͽ�����", ip.c_str(), port);

	return;
}

void Worker::OnMsg(mdk::STNetHost &host)
{
	msg::Buffer buffer; 
	if ( !host.Recv(buffer, buffer.HeaderSize(), false) ) return;
	if ( !buffer.ReadHeader() )
	{
		if ( !host.IsServer() ) host.Close();
		m_log.Info("Error","����ͷ����");
		return;
	}
	if ( !host.Recv(buffer, buffer.Size()) ) return;

	//���ģ������
	if ( Moudle::gmap != buffer.MoudleId() ) 
	{
		m_log.Info("Error","δԤ�ϵ�ģ��");
		host.Close();
		return;
	}

	if ( buffer.IsResult() ) //clientֻ�ܷ�����
	{
		m_log.Info("Error","δԤ�ϵ�Ӧ��");
		host.Close();
		return;
	}

	//��������
	switch ( buffer.Id() )
	{
	case MsgId::newId ://����һ��Ψһid
		OnNewId(host, buffer);
		break;
	case MsgId::setPoint ://���ö���
		OnSetPoint(host, buffer);
		break;
	case MsgId::addLineForPoint ://��������ӱ�
		OnAddLineForPoint(host, buffer);
		break;
	case MsgId::delLineForPoint ://ɾ������ı�
		OnDelLineForPoint(host, buffer);
		break;
	case MsgId::getPoint ://ȡ�ö���
		OnGetPoint(host, buffer);
		break;
	case MsgId::addLine ://������
		OnAddLine(host, buffer);
		break;
	case MsgId::delLine ://ɾ����
		OnDelLine(host, buffer);
		break;
	case MsgId::getLine ://ȡ�ñ�
		OnGetLine(host, buffer);
		break;
	case MsgId::delPoint ://ɾ������
		OnDelPoint(host, buffer);
		break;
	case MsgId::setMoveOpt ://����move
		OnSetMoveOpt(host, buffer);
		break;
	case MsgId::move://�༶�ƶ�
		OnMove(host, buffer);
		break;
	case MsgId::setPoints://�������ö���
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
		m_log.Info("Error", "����idʧ�ܣ��޷���������");
		host.Close();
		return true;
	}

	Grid::Result::Result ret = m_grid.CreateId(msg.m_id, msg.m_count);
	if ( Grid::Result::fileError == ret )
	{
		msg.m_reason = "�޷�����id";
		msg.m_code = ResultCode::refuse;
		m_log.Info("Error", "����idʧ�ܣ�%s", msg.m_reason.c_str() );
	}
	if ( Grid::Result::noIdSource == ret )
	{
		msg.m_reason = "�޿���id��Դ";
		msg.m_code = ResultCode::refuse;
		m_log.Info("Error", "����idʧ�ܣ�%s", msg.m_reason.c_str() );
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
		m_log.Info("Error", "���ö���ʧ�ܣ��޷���������");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.SetPoint(msg.m_point);
	msg.m_point.Release();//�ͷ��ڴ�
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "���ö���ʧ�ܣ�%s", msg.m_reason.c_str() );
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
		m_log.Info("Error", "��������ӱ�ʧ�ܣ��޷���������");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.AddLineForPoint(msg.m_pointId, msg.m_lineId, msg.m_isOut);
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "��������ӱ�ʧ�ܣ�%s", msg.m_reason.c_str() );
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
		m_log.Info("Error", "ɾ������ı�ʧ�ܣ��޷���������");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.DelLineForPoint(msg.m_pointId, msg.m_lineId, msg.m_isOut);
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "ɾ������ı�ʧ�ܣ�%s", msg.m_reason.c_str() );
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
		m_log.Info("Error", "ȡ����ʧ�ܣ��޷���������");
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
		m_log.Info("Error", "���Ӷ���ʧ�ܣ��޷���������");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.AddLine(msg.m_line);
	msg.m_line.Release();//�ͷ��ڴ�
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "���Ӷ���ʧ�ܣ�%s", msg.m_reason.c_str() );
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
		m_log.Info("Error", "ɾ����ʧ�ܣ��޷���������");
		host.Close();
		return true;
	}

	CallResult ret = m_grid.DelLine(msg.m_lineId, msg.m_startId, msg.m_endId);
	if ( !ret.isSuccess )
	{
		msg.m_reason = ret.reason;
		msg.m_code = (ResultCode::ResultCode)ret.code;
		m_log.Info("Error", "ɾ����ʧ�ܣ�%s", msg.m_reason.c_str() );
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
		m_log.Info("Error", "��ѯ��ʧ�ܣ��޷���������");
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
		m_log.Info("Error", "ɾ������ʧ�ܣ��޷���������");
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
		m_log.Info("Error", "���ò�ѯʧ�ܣ��޷���������");
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
		m_log.Info("Error", "��ѯʧ�ܣ��޷���������");
		host.Close();
		return true;
	}

	std::map<int, MOVE_OPT>::iterator it = m_moveOpts.find(host.ID());
	if ( it == m_moveOpts.end() ) 
	{
		msg.m_reason = "û�г�ʼ����ѯ����";
		msg.m_code = ResultCode::refuse;
		m_log.Info("Error", "�ܾ���ѯ��%s", msg.m_reason.c_str() );
		msg.Build( true );
		host.Send(msg, msg.Size());
		return true;
	}

	MOVE_OPT &opt = it->second;//�õ���ѯ����,�������
	int i = 0;
	int idsSize = msg.m_ids.size();
	for ( i = 0; i < idsSize; i++ ) opt.ids.push_back(msg.m_ids[i]);

	if ( !msg.m_moveStart ) return true;//����δ������ɣ��ȴ�����

	if ( 0 == opt.selectLines.size() )
	{
		if (Grid::unlink == opt.act)
		{
			msg.m_reason = "unlink���������ƶ�";
			msg.m_code = ResultCode::refuse;
			m_log.Info("Error", "�ܾ���ѯ��%s", msg.m_reason.c_str() );
			msg.Build( true );
			host.Send(msg, msg.Size());
			return true;
		}
		if (msg.m_fromPoint && !msg.m_filterStartPoint 
			&& Grid::count == opt.act )
		{
			msg.m_reason = "count���������ƶ� or ���ù�������";
			msg.m_code = ResultCode::refuse;
			m_log.Info("Error", "�ܾ���ѯ��%s", msg.m_reason.c_str() );
			msg.Build( true );
			host.Send(msg, msg.Size());
			return true;
		}
	}
	if ( Grid::del == opt.act || Grid::unlink == opt.act )
	{
		if ( 1 < opt.selectLines.size() )
		{
			msg.m_reason = "unlink��del������֧�ֶ༶�ƶ�";
			msg.m_code = ResultCode::refuse;
			m_log.Info("Error", "�ܾ���ѯ��%s", msg.m_reason.c_str() );
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
		m_log.Info("Error", "��ѯʧ�ܣ�%s", msg.m_reason.c_str() );
		msg.Build(true);
		host.Send(msg, msg.Size());
		return true;
	}

	//�����յ�
	int startPos = 0;
	int sendCount = 0;
	while ( startPos < m_endPoints.size )
	{
		replyData.Build(m_endPoints.datas, m_endPoints.size, startPos, opt.selectAll, opt.getFields);
		host.Send(replyData, replyData.Size());
		sendCount++;
	}

	//������Ҫ�������ڵ��ѯ�ı����м䶥��
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

	//���ͳɹ�֪ͨ
	msg.m_code = ResultCode::success;
	msg.Build(true);
	host.Send(msg, msg.Size());
	sendCount++;
// 	startMove = mdk::MillTime() - startMove;
// 	m_log.Info("Debug", "��ѯ��ʱ��%lld����", startMove);
	 
	return true;
}

bool Worker::OnSetPoints( mdk::STNetHost &host, msg::Buffer &buffer )
{
	msg::MsgSetPoints msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "�������ö���ʧ�ܣ��޷���������");
		host.Close();
		return true;
	}
	std::vector<Grid::Point> &points = m_points[host.ID()];//ɾ�����������Ķ����б�
	int i = 0;
	for ( ; i < msg.m_points.size(); i++ )
	{
		points.push_back(msg.m_points[i]);
	}
	if ( !msg.m_isEnd )//�б�δ��ɣ��ȴ�����
	{
		return true;
	}

	if ( msg.m_isCreate )//�½���������д�뵽�ļ�ĩβ
	{
		CallResult ret = m_grid.CreatePoints(points);
		if ( !ret.isSuccess )
		{
			msg.m_code = (ResultCode::ResultCode)ret.code;
			msg.m_reason = ret.reason;
		}
	}
	else//�༭���㣬������������
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
