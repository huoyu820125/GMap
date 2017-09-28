#ifndef SERVER_EX_H
#define SERVER_EX_H

#include "mdk/include/frame/netserver/NetServer.h"
#include "mdk/include/mdk/Signal.h"
#include "mdk/include/mdk/Lock.h"
#include "mdk/include/mdk/Executor.h"
#include "mdk/include/mdk/ConfigFile.h"
#include "Console.h"

#include <map>
#include <vector>
#include <string>

namespace mdk
{

class ServerEx : public NetServer
{
public:
	ServerEx(void);
	virtual ~ServerEx(void);

	//////////////////////////////////////////////////////////////////////////
	//业务线程独占对象
public:
	//设置工作线程数量
	void SetWorkThreadCount(int nCount);
	/*
		设置工作线程访问对象，让每个工作线程独占一个对象
		objList元素数量必须与工作线程数量相同，确保每个工作线程1个对象不多不少
		不调用SetWorkThreadCount，默认是16个工作线程
		本方法调用，不可再调用SetWorkThreadCount,否则立刻断言崩溃

		objList里面的对象必须保证在~ServerEx()前都不会被释放
		如果是new出来的，需要传入freeFun和pObj参数，告诉~ServerEx如何释放对象
		例如：
		class AB
		{
			public:
			AB(){}
			virtual ~AB(){}
			void* RemoteCall freeData(void *p)
			{
				A *pA = (A*)p;
				delete pA;

				return NULL;
			}
		};

		AB *p = new AB();
		SetThreadsObjects(a, mdk::Executor::Bind(&AB::freeData), p);
	*/
	void SetThreadsObjects( const std::vector<void*> &objList, 
							MethodPointer freeFun = NULL, void *pObj = NULL );

	//返回调用工作线程独占的对象
	void* SafeObject();

protected:
	//为一个线程分配安全对象
	void AllocSafeObject();

private:
	void* RemoteCall OnWorkStart(void*);

private:
	int m_workThreadCount;//业务线程数量
	std::map<uint64, void*> m_TObjectMap;//工作线程ID——独占对象映射
	std::vector<void*> m_objects;//分配个工作线程的对象
	std::vector<void*>::iterator m_nextObject;//下一个未分配对象
	Mutex	m_startLock;//OnWorkStart锁
	Signal	m_sigFinished;//业务线程池启动完成
	bool	m_threadObjSeted;//线程对象已设置
	//线程对象释放方法
	MethodPointer m_freeFun;
	void *m_pFreeFunObj;

	//////////////////////////////////////////////////////////////////////////
	//分布式生成唯一id
protected:
	/*
		使用说明
		按照OnlyId.cfg配置结点唯一Id
		服务启动前调用1次InitId(配置文件名)
		以后每次调用CreateId()就可以产生集群中全局唯一id
		调用SaveId()立即将下次可用id写入磁盘
	*/
	//初始化id
	void InitId(const std::string &fileName);
	//创建全局唯一id,并调用SaveId()定时写磁盘
	bool CreateId( uint64 &id );
	//写磁盘
	//now=true立即写
	//now=false 每使用10万个id,或上次创建id距离本次创建超过5秒,会写1次磁盘
	void SaveId( bool now = true );

private:
	uint16 m_nodeId;
	uint64 m_searialNo;
	ConfigFile *m_pCfg;

	//////////////////////////////////////////////////////////////////////////
	//命令行输入支持
public:
	void SetPrompt( char *prompt );//设置提示符，可以在任何时候调用，为了修改提示符
	void StartCommand();//开始接受命令行输入
	void StopCommand();//停止命令行

protected:
	char* RemoteCall CommandAcceptor(std::vector<std::string> *cmd);
	virtual char* OnCommand(const std::vector<std::string> &cmd);

private:
	Console	m_console;
};

}

#endif	//SERVER_EX_H
