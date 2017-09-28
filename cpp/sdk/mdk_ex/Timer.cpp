// Timer.cpp: implementation of the Timer class.
//
//////////////////////////////////////////////////////////////////////

#ifdef WIN32
#include "windows.h"
#endif
#include "Timer.h"
#include <ctime>
#include<vector>
#include "mdk/include/mdk/atom.h"
#include <cstring>
#include <cstdio>
#include "ShareObject.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
namespace mdk
{
	Timer::Timer()
	{
		if ( m_worker.Run( Executor::Bind(&Timer::Work), this, NULL ) )
		{
			m_sigRun.Wait();
		}
	}

	Timer::~Timer()
	{

	}

	void Timer::SetTimer( int eventId, uint64 second, MethodPointer method, void *pObj, ShareObject *pSafeObj, void *pData, int dataSize, int repeat )
	{
		KillTimer( eventId );
		TASK* pTask = new TASK;
		pTask->ref = 1;
		pTask->eventId = eventId;
		pTask->lastTime = time(NULL);
		pTask->millSecond = second * 1000;
		pTask->repeat = repeat;
		pTask->state = 0;
		if ( NULL == pSafeObj ) pTask->pObj = (ShareObject*)pObj;
  		else pTask->pObj = pSafeObj;
  		pTask->pObj->AddRef();
		pTask->dataSize = dataSize;
		pTask->pData = NULL;
		if ( pTask->dataSize > 0 )
		{
			pTask->pData = new char[pTask->dataSize];
			memcpy( pTask->pData, pData, pTask->dataSize);
		}
		pTask->worker.Accept( method, pObj, pTask->pData );

		AutoLock lock(&m_lock);
		m_waitTasks.insert(std::map<int, TASK*>::value_type(eventId, pTask));

		return;
	}

	void Timer::SetTimer( int eventId, uint64 second, FuntionPointer fun, void *pData, int dataSize, int repeat )
	{
		KillTimer( eventId );
		TASK* pTask = new TASK;
		pTask->ref = 1;
		pTask->eventId = eventId;
		pTask->lastTime = time(NULL);
		pTask->millSecond = second * 1000;
		pTask->repeat = repeat;
		pTask->state = 0;
		pTask->pObj = NULL;
		pTask->dataSize = dataSize;
		pTask->pData = NULL;
		if ( pTask->dataSize > 0 )
		{
			pTask->pData = new char[pTask->dataSize];
			memcpy( pTask->pData, pData, pTask->dataSize);
		}
		pTask->worker.Accept( fun, pTask->pData );

		AutoLock lock(&m_lock);
		m_waitTasks.insert(std::map<int, TASK*>::value_type(eventId, pTask));

		return;
	}

	void Timer::KillTimer( int eventId )
	{
		uint64 threadId = CurThreadId();
		TASK* pTask = NULL;
		{
			AutoLock lock(&m_lock);
			std::map<int, TASK*>::iterator it = m_waitTasks.find( eventId );
			if ( m_waitTasks.end() == it ) return;
			pTask = it->second;
			m_waitTasks.erase( it );
		}
		if ( threadId != m_threadId) 
		{
			AutoLock lock(&pTask->lock);
			pTask->state = 2;
		}
		pTask->Release();
		return;
	}

	void Timer::TASK::Release()
	{
		if ( 1 != AtomDec(&ref, 1) ) 
		{
			return;
		}
		pObj->Release();
		if ( NULL != pData ) 
		{
			delete[]pData;
			pData = NULL;
		}

		delete this;
	}

	void Timer::KillAllTimer()
	{
		std::vector<int> allIds;
		{//����idȫ�����Ƶ�allIds��
			AutoLock lock(&m_lock);
			std::map<int, TASK*>::iterator it = m_waitTasks.begin();
			while ( it != m_waitTasks.end() ) 
			{
				allIds.push_back(it->first);
				it++;
			}
		}

		//�������KillTimer
		unsigned int i = 0;
		for ( ; i < allIds.size(); i++ )
		{
			KillTimer(allIds[i]);
		}

		return;
	}

#ifdef WIN32
#else
	static uint64 GetTickCount()
	{
		struct timespec ts;

		clock_gettime(CLOCK_MONOTONIC, &ts);

		return (ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
	}
#endif

	void* Timer::Work(void *param)
	{
		m_threadId = CurThreadId();
		m_sigRun.Notify();
		uint64 curtime;
		std::vector<TASK*> waitTasks;
		while ( true )
		{
			waitTasks.clear();
			curtime = GetTickCount();
			{//��ʱ�������ȫ�����浽waitTasks�����
				AutoLock lock(&m_lock);
				std::map<int, TASK*>::iterator it = m_waitTasks.begin();
				while ( it != m_waitTasks.end() ) 
				{
					if ( curtime - it->second->lastTime < it->second->millSecond ) 
					{
						it++;
						continue;
					}
					it->second->lastTime = curtime;
					AtomAdd(&it->second->ref, 1);
					waitTasks.push_back(it->second);
					if ( 0 < it->second->repeat ) 
					{
						if ( 1 == it->second->repeat ) 
						{
							it->second->Release();
							m_waitTasks.erase(it++);
						}
						else it->second->repeat--;
					}
					else it++;
				}
			}
			{//�������ִ��
				unsigned int i = 0;
				for ( ; i < waitTasks.size(); i++ )
				{
					{
						AutoLock lock(&waitTasks[i]->lock);//������SetTimer�̣߳�ֻ��������KillTimer�߳�
						if ( 2 != waitTasks[i]->state ) 
						{
							waitTasks[i]->worker.Execute();
						}
					}
					waitTasks[i]->Release();
				}
			}
			m_sleep( 1000 );
		}

		return NULL;
	}

}
