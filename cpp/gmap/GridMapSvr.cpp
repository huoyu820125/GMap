// GridMapSvr.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "Worker.h"

int main(int argc, char* argv[])
{
	if ( 2 > argc ) 
	{
		printf( "����ָ�gmap [����·���������ļ����������ļ�������gmap����ͬ�ļ�����]\n" );
		return 0;
	}

	Worker svr(argv[1]);
	const char *reason = svr.Start();
	if ( NULL != reason )
	{
		svr.m_log.Info( "Error", "��������:%s", reason );
		return 0;
	}
	svr.WaitStop();

	return 0;
}
