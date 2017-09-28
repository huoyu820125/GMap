// ClusterCgCli.cpp : 定义控制台应用程序的入口点。
//

#ifdef WIN32
#ifdef _DEBUG
#pragma comment ( lib, "mdk_d.lib" )
//#pragma comment ( lib, "hiredis_d.lib" )
#else
#pragma comment ( lib, "mdk.lib" )
// #pragma comment ( lib, "hiredis.lib" )
#endif
#endif

#include "..\Client.h"

int main(int argc, char* argv[])
{
	Client cli;
	cli.SetService("192.168.1.229", 8888);
	std::map<NetLine::NetLine, std::vector<NODE> > cluster; 
	cli.GetServices(cluster, Moudle::userSvr);
	return 0;
}

