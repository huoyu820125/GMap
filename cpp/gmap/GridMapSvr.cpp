// GridMapSvr.cpp : 定义控制台应用程序的入口点。
//

#include "Worker.h"

int main(int argc, char* argv[])
{
	if ( 2 > argc ) 
	{
		printf( "运行指令：gmap [不带路径的配置文件名，配置文件必须与gmap在相同文件夹下]\n" );
		return 0;
	}

	Worker svr(argv[1]);
	const char *reason = svr.Start();
	if ( NULL != reason )
	{
		svr.m_log.Info( "Error", "启动错误:%s", reason );
		return 0;
	}
	svr.WaitStop();

	return 0;
}
