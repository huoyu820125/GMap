// Timer.h: interface for the Timer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef TIMER_H
#define TIMER_H

#include "mdk/include/mdk/FixLengthInt.h"
#include "mdk/include/mdk/Thread.h"
#include "mdk/include/mdk/Lock.h"
#include "mdk/include/mdk/Signal.h"
#include "mdk/include/mdk/mapi.h"

#include <map>
namespace mdk
{
	class ShareObject;
	class Timer  
	{
	public:
		Timer();
		virtual ~Timer();
		/*
			��������ָ��һ��ת������void*�����޷�ת������ȷ�Ļ���ָ���ַ
			���Ա���Ҫ��ShareObject�������ն������ָ�룬���������Ա
			
			pObj��һ��ShareObject�������࣬��ȷ��pObjָ��Ķ��󲻻��ڶ�ʱ���˳�ǰ���ͷ�
		*/
		void SetTimer( int eventId, uint64 second, MethodPointer method, void *pObj, ShareObject *pSafeObj = NULL, void *pData = NULL, int dataSize = 0, int repeat = -1);
		void SetTimer( int eventId, uint64 second, FuntionPointer fun, void *pData = NULL, int dataSize = 0, int repeat = -1 );
		void KillTimer( int eventId );
		void KillAllTimer();

		void* RemoteCall Work(void *param);
	private:
		Thread m_worker;

		typedef struct TASK
		{
			int eventId;
			Task worker;
			uint64 millSecond;
			uint64	lastTime;
			int repeat;
			int state;
			Mutex lock;
			void Release();
			int ref;
			ShareObject *pObj;
			void *pData;
			int dataSize;
		}TASK;
		std::map<int, TASK*> m_waitTasks;
		Mutex m_lock;
		Signal m_sigRun;
		uint64 m_threadId;
	};


}

#endif // !defined TIMER_H
