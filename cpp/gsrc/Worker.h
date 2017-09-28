#pragma once

#include "mdk_ex/ServerEx.h"
#include "mdk/include/frame/netserver/HostData.h"
#include "mdk/include/mdk/Logger.h"
#include "mdk/include/mdk/ConfigFile.h"
#include "mdk/include/mdk/Lock.h"
#include <vector>
#include <string>
#include "protocl/cpp/Buffer.h"

class Worker : public mdk::ServerEx
{
	friend int main(int argc, char* argv[]);
public:
	Worker(void);
	virtual ~Worker(void);

	virtual void OnConnect(mdk::NetHost &host);
	virtual void OnCloseConnect(mdk::NetHost &host);
	virtual void OnMsg(mdk::NetHost &host);

	//////////////////////////////////////////////////////////////////////////
	//配置模块
	//取集群信息
	bool OnGetCluster(mdk::NetHost &host, msg::Buffer &buffer);
	//取得clientId
	bool OnGetClientId(mdk::NetHost &host, msg::Buffer &buffer);

private:

private:
	mdk::Logger		m_log;
	mdk::ConfigFile	m_cfg;
	mdk::int32		m_nodeId;//结点编号
	char			m_cfgFile[2048];
};

