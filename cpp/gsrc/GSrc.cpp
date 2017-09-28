#include "Worker.h"
#include "mdk/include/mdk/mapi.h"

int main(int argc, char* argv[])
{
	Worker svr;
	const char *reason = svr.Start();
	if ( NULL != reason )
	{
		svr.m_log.Info( "Error", "Æô¶¯´íÎó:%s", reason );
		return 0;
	}
	svr.WaitStop();

	return 0;
}

