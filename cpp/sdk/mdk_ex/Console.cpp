// Console.cpp: implementation of the Console class.
//
//////////////////////////////////////////////////////////////////////

#include "Console.h"
#include <cstring>
#include <cstdio>
using namespace std;

namespace mdk
{

static void split( vector<string> &des, const char *str,char *spliter )
{
	des.clear();
	string temp = str;
	char *item;
	item = strtok( (char*)temp.data(), spliter );  
	while( item != NULL )  
	{    
		des.push_back(item);
		item = strtok( NULL, spliter );  
	}
	temp = "";
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Console::Console(char *prompt, int cmdMaxSize)
{
	m_bRun = false;
	if ( NULL == prompt ) m_prompt = "Input Command";
	else m_prompt = prompt;
	m_cmdMaxSize = (1024>cmdMaxSize?1025:cmdMaxSize + 1);
}

Console::~Console()
{

}

bool Console::Start( MethodPointer method, void *pObj )
{
	m_bRun = true;

	m_isMemberMethod = true;
	m_method = method;
	m_pObj = pObj;
	m_fun = NULL;
	return m_inputThread.Run(Executor::Bind(&Console::InputThread), this, NULL);
}

bool Console::Start( FuntionPointer fun )
{
	m_bRun = true;
	m_isMemberMethod = false;
	m_method = 0;
	m_pObj = NULL;
	m_fun = fun;
	return m_inputThread.Run(Executor::Bind(&Console::InputThread), this, NULL);
}

void Console::Stop()
{
	m_bRun = false;
	m_inputThread.Stop( 3000 );
}

void Console::WaitStop()
{
	m_inputThread.WaitStop();
}

void* Console::InputThread(void*)
{
	char *cmd = new char[m_cmdMaxSize];
	if ( NULL == cmd ) return NULL;
	//接受用户指令输入
	int cmdSize = 0;
	bool valid = true;
	char *ret = NULL;
	while( m_bRun )
	{
		valid = true;
#ifdef WIN32
		printf( "%s:\\>", m_prompt.c_str() );
#else
		printf( "%s>", m_prompt.c_str() );
#endif

		while ( fgets(cmd, m_cmdMaxSize, stdin) )
		{

			cmdSize = strlen( cmd );
			if ( '\n' == cmd[cmdSize-1] ) break;
			valid = false;
		}
		if ( !m_bRun ) return NULL; 
		if ( !valid ) 
		{
			PrintReulst("Invalid command");
			continue;
		}
		PrintReulst( ExecuteCommand(cmd) );
	}

	return NULL;
}

char* Console::ExecuteCommand(char *cmd)
{
	split( m_cmd, cmd, " \t\n" );
	if ( 0 >= m_cmd.size() ) return "";
	if ( m_isMemberMethod ) return (char*)Executor::CallMethod(m_method, m_pObj, &m_cmd);
	else return (char*)m_fun(&m_cmd);
}

void Console::PrintReulst( const char* info )
{
	if ( NULL != info && '\0' != info[0] )
	{
			printf( "%s\n", info );
			fflush(stdout);
	}
}

void Console::SetPrompt( char *prompt )
{
	if ( NULL == prompt ) m_prompt = "Input Command";
	else m_prompt = prompt;
}

}