#include "ServerEx.h"

#include "mdk/include/mdk/mapi.h"

namespace mdk
{

ServerEx::ServerEx(void)
	:m_console("svr", 1024)
{
	m_workThreadCount = 16;
	m_threadObjSeted = false;
	m_freeFun = NULL;
	m_pFreeFunObj = NULL;
	m_pCfg = NULL;
}

ServerEx::~ServerEx(void)
{
	SaveId();
	if ( NULL != m_pCfg )
	{
		delete m_pCfg;
		m_pCfg = NULL;
	}
	if ( NULL != m_freeFun )
	{
		int i = 0;
		for ( i = 0; i < m_workThreadCount; i++ )
		{
			Executor::CallMethod(m_freeFun, m_pFreeFunObj, m_objects[i]);
		}
	}
}

void ServerEx::SetWorkThreadCount(int nCount)
{
	//���뻹û�е���SetThreadsObjects()
	mdk_assert(!m_threadObjSeted);
	m_workThreadCount = nCount;
	NetServer::SetWorkThreadCount( m_workThreadCount );
}

void ServerEx::SetThreadsObjects( const std::vector<void*> & objList, MethodPointer freeFun, void *pObj )
{
	//�����߳��������ռ��������������ͬ
	mdk_assert( objList.size() >= m_workThreadCount ); 
	int i = 0;
	for ( i = 0; i < objList.size(); i++ )
	{
		m_objects.push_back(objList[i]);
	}
	m_nextObject = m_objects.begin();
	SetOnWorkStart( Executor::Bind(&ServerEx::OnWorkStart), this, NULL );
	m_threadObjSeted = true;
	m_freeFun = freeFun;
	m_pFreeFunObj = pObj;
}

void* ServerEx::OnWorkStart(void*)
{
	AllocSafeObject();
// 	uint64 tid = CurThreadId();
// 	AutoLock lock(&m_startLock);
// 	m_TObjectMap[tid] = *m_nextObject;//Ϊ��ǰ�̷߳���һ����ռ����
// 	m_nextObject++;//ָ����һ��δ����Ķ�ռ����
// 	//�������δ�����ȥ�Ķ�ռ���󣬵ȴ������̷߳���
// 	if ( m_nextObject != m_objects.end() ) 
// 	{
// 		lock.Unlock();
// 		m_sigFinished.Wait();
// 	}
// 	//֪ͨ��һ���߳����ж����Ѿ��������
// 	m_sigFinished.Notify();
	
	return NULL;
}

void* ServerEx::SafeObject()
{
	uint64 tid = CurThreadId();
	return m_TObjectMap[tid];
}

void ServerEx::InitId( const std::string &fileName )
{
	char exePath[1024];
	GetExeDir( exePath, 1024 );//ȡ�ÿ�ִ�г���λ��
	char cfgFile[1024+256];
	sprintf( cfgFile, "%s/%s", exePath, fileName.c_str() );
	m_pCfg = new ConfigFile();

	ConfigFile &cfg = *m_pCfg;
	cfg.ReadConfig( cfgFile );
	int nodeId = cfg["Only Id"]["nodeId"];
	std::string searialNo = cfg["Only Id"]["searialNo"];
	m_nodeId = nodeId;
	m_searialNo = 0;
	sscanf( searialNo.c_str(), "%lld", &m_searialNo );

	return;
}

bool ServerEx::CreateId( uint64 &id )
{
	//id������
	if ( 0x0001000000000000ull == m_searialNo ) return false;

	id = m_nodeId;
	id <<= 48;
	id += m_searialNo;
	m_searialNo++;
	SaveId(false);

	return true;
}

void ServerEx::SaveId( bool now )
{
	static uint64 count = 0;
	static uint64 lastSave = 0;
	if ( NULL == m_pCfg ) return;
	ConfigFile &cfg = *m_pCfg;

	count++;
	if ( now || 0 == count % 10000
		|| MillTime() - lastSave > 5000 )
	{
		lastSave = MillTime();
		char temp[128];
		sprintf( temp, "%lld", m_searialNo );
		cfg["Only Id"]["searialNo"] = (std::string)temp;
		cfg.Save();
	}
}


void ServerEx::SetPrompt( char *prompt )
{
	m_console.SetPrompt(prompt);
}

void ServerEx::StartCommand()
{
	m_console.Start(Executor::Bind(&ServerEx::CommandAcceptor), this);
}

void ServerEx::StopCommand()
{
	m_console.Stop();
}

char* ServerEx::CommandAcceptor(std::vector<std::string> *cmd)
{
	return OnCommand(*cmd);
}

char* ServerEx::OnCommand(const std::vector<std::string> &cmd)
{
	return "����ָ����Ҫ���ػص�������virtual char* OnCommand(const std::vector<std::string> &cmd)";
}

void ServerEx::AllocSafeObject()
{
	uint64 tid = CurThreadId();
	AutoLock lock(&m_startLock);
	m_TObjectMap[tid] = *m_nextObject;//Ϊ��ǰ�̷߳���һ����ռ����
	m_nextObject++;//ָ����һ��δ����Ķ�ռ����
	//�������δ�����ȥ�Ķ�ռ���󣬵ȴ������̷߳���
	if ( m_nextObject != m_objects.end() ) 
	{
		lock.Unlock();
		m_sigFinished.Wait();
	}
	//֪ͨ��һ���߳����ж����Ѿ��������
	m_sigFinished.Notify();
}

}

