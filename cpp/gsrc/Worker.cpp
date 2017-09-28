#include "Worker.h"
#include "mdk/include/mdk/mapi.h"
#include <cstring>
#include <cstdlib>

#include "protocl/cpp/object/gsrc/GetCluster.h"
#include "protocl/cpp/object/gsrc/MsgGetClientId.h"

#include "mdk_ex/File.h"
#ifdef WIN32
#ifdef _DEBUG
#pragma comment ( lib, "mdk_d.lib" )
#else
#pragma comment ( lib, "mdk.lib" )
#endif
#endif

Worker::Worker(void)
{
	m_log.SetLogName("gsrc");
	m_log.SetPrintLog(true);

	char exeDir[256];
	mdk::GetExeDir( exeDir, 256 );//ȡ�ÿ�ִ�г���λ��
	sprintf( m_cfgFile, "%s/gsrc.cfg", exeDir );
	m_log.Info("Run", "��ȡ���ã�%s", m_cfgFile);
	if ( !m_cfg.ReadConfig( m_cfgFile ) )
	{
		m_log.Info( "Error", "���ô���" );
		mdk::mdk_assert(false);
		exit(0);
	}
	std::string ip = m_cfg["opt"]["ip"];
	int port = m_cfg["opt"]["listen"];
	int CPU = mdk::GetCUPNumber(32, 32);
	SetWorkThreadCount(CPU);

	m_log.Info( "Run", "�����˿�:%d", port );
	OpenNoDelay();
	Listen(port);
}

Worker::~Worker(void)
{
}

void Worker::OnConnect(mdk::NetHost &host)
{
	std::string ip;
	int port;
	if ( !host.IsServer() ) host.GetAddress(ip, port);
	if ( host.IsServer() ) host.GetServerAddress(ip, port);
	m_log.Info("Run", "client(%s:%d):����", ip.c_str(), port);
}

void Worker::OnCloseConnect(mdk::NetHost &host)
{
	std::string ip;
	int port;
	if ( !host.IsServer() ) host.GetAddress(ip, port);
	if ( host.IsServer() ) host.GetServerAddress(ip, port);
	m_log.Info("Run", "client(%s:%d)�Ͽ�", ip.c_str(), port);
}

void Worker::OnMsg(mdk::NetHost &host)
{
	msg::Buffer buffer; 
	if ( !host.Recv(buffer, buffer.HeaderSize(), false) ) return;
	if ( !buffer.ReadHeader() )
	{
		if ( !host.IsServer() ) host.Close();
		m_log.Info("Error","�Ƿ�����");
		return;
	}
	if ( !host.Recv(buffer, buffer.Size()) ) return;

	//���ģ������
	if ( Moudle::gsrc != buffer.MoudleId() ) 
	{
		m_log.Info("Error","�Ƿ�����");
		host.Close();
		return;
	}

	if ( buffer.IsResult() ) //clientֻ�ܷ�����
	{
		m_log.Info("Error","�Ƿ�Ӧ��");
		host.Close();
		return;
	}

	//��������
	switch ( buffer.Id() )
	{
	case MsgId::getCluster :
		OnGetCluster(host, buffer);
		break;
	case MsgId::getClientId :
		OnGetClientId(host, buffer);
		break;
	default:
		break;
	}
	return;
}

bool Worker::OnGetCluster(mdk::NetHost &host, msg::Buffer &buffer)
{
	msg::GetCluster msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "����ȡ��Ⱥ������ʧ��");
		host.Close();
		return true;
	}

	mdk::ConfigFile	cfg;
	if ( !cfg.ReadConfig( m_cfgFile ) )
	{
		m_log.Info("Error", "��ȡ��Ⱥ����ʧ�ܣ���������");
		return true;
	}
	int val;
	NetLine::NetLine line;
	NODE node;
	int i = 0;
	int count = cfg["cluster info"]["node count"];
	char paramName[256];
	int maxCount = 2;
	for ( i = msg.m_start; maxCount > 0 && i <= count; maxCount--, i++ )
	{
		node.nodeId = i;
		sprintf(paramName, "ip%d", i);
		node.ip = (std::string)cfg["cluster info"][paramName];
		sprintf(paramName, "port%d", i);
		node.port = cfg["cluster info"][paramName];
		sprintf(paramName, "net work%d", i);
		line = (NetLine::NetLine)(int)cfg["cluster info"][paramName];
		msg.m_cluster[line].push_back(node);
	}
	if ( !msg.Build(true) ) 
	{
		m_log.Info("Error", "���챨��ʧ��");
		return false;
	}
	host.Send(msg, msg.Size());

	return true;
}

bool NewClientId(int64 &clientId, std::string &reason)
{
	//client = 24b(index:0~16777215)
	//searchId = 24b(clientId:0~16777215) + 22b(index��0~4194303)
	char exeDir[256];
	mdk::GetExeDir( exeDir, 256 );//ȡ�ÿ�ִ�г���λ��
	mdk::File file(exeDir, "clientId");
	mdk::File::Error ret = file.Open(mdk::File::read, mdk::File::assii);
	int64 nextId;
	if ( mdk::File::success != ret ) 
	{
		if ( mdk::File::fileNoExist != ret ) 
		{
			reason = "client id��������";
			return false;
		}
		clientId = 0;//�ļ������ڣ���һ�δ�����0��ʼ
	}
	else
	{
		if ( mdk::File::success != file.Read(&clientId, sizeof(int64)) )
		{
			reason = "client id��������";
			file.Close();
			return false;
		}
		file.Close();
	}
	if ( 16777215 < clientId ) 
	{
		reason = "�޿���client id";
		return false;
	}
	nextId = clientId + 1;
	clientId = clientId << 22;
	reason = "���ܼ�¼����";
	if ( mdk::File::success != file.Open(mdk::File::write, mdk::File::assii) ) return false;
	if ( mdk::File::success != file.Write(&nextId, sizeof(int64)) ) return false;
	file.Close();

	return true;
}

bool Worker::OnGetClientId(mdk::NetHost &host, msg::Buffer &buffer)
{
	msg::MsgGetClientId msg;
	memcpy(msg, buffer, buffer.Size());
	if ( !msg.Parse() )
	{
		m_log.Info("Error", "����client idʧ�ܣ��޷���������");
		host.Close();
		return true;
	}
	if ( !NewClientId(msg.m_clientId, msg.m_reason) ) msg.m_code = ResultCode::refuse;
	msg.Build(true);
	host.Send(msg, msg.Size());

	return true;
}
