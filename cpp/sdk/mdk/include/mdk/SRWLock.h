// SRWLock.h: interface for the SRWLock class.
//
//////////////////////////////////////////////////////////////////////

#ifndef SRWLOCK_H
#define SRWLOCK_H
#include "FixLengthInt.h"
#include "Lock.h"
#include "Signal.h"

/*
	linux32�µĽ��̿�ӵ�е��߳�����������1024
	ʵ����32λ���û��ռ���3G��Ҳ����3072M��3072M����ÿ���߳�ջ��С8M��384��
	����һ���������ӵ��384���̡߳�
	��ʹ�� ulimit -s 1024 ��СĬ�ϵ�ջ��С��Ҳ�޷�ͻ��1024���̵߳�Ӳ���ƣ��������±��� C ��
   
	windowsϵͳ��������ഴ�����߳�����λ2000
 */
#define MAX_THREAD_COUNT 2000	//����߳�����

class SRWLock  
{
public:
	SRWLock();
	virtual ~SRWLock();

	void Lock();
	void Unlock();
	
	void ShareLock();
	void ShareUnlock();
	
protected:
	typedef struct THREAD_HIS
	{
		mdk::uint64 threadID;
		mdk::uint16 readCount;
		mdk::uint16 writeCount;
	}THREAD_HIS;
	SRWLock::THREAD_HIS* GetThreadData( bool lock );
		
private:
	THREAD_HIS m_threadData[MAX_THREAD_COUNT];
	mdk::uint32 m_readCount;
	mdk::uint32 m_writeCount;
	mdk::Mutex m_lock;
	mdk::int32 m_waitWriteCount;
	mdk::Signal m_waitWrite;
	mdk::Mutex m_checkLock;
	mdk::uint64 m_ownerThreadId;
	mdk::Signal m_waitRead;
	bool m_bWaitRead;

};

#endif // ifndef(SRWLOCK_H)
