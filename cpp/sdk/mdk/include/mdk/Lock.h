// Lock.h: interface for the Thread class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MDK_LOCK_H
#define MDK_LOCK_H

#ifdef WIN32
#else
#include <pthread.h>
#endif

namespace mdk
{

#ifdef WIN32
//Ϊ�˲�����windows.h,����һ�����Դ���CRITICAL_SECTION��buffer
typedef struct CS_BUFFER
{
	char	buffer[64];
}CS_BUFFER;
#define OriginalMutex CS_BUFFER
#else
	typedef pthread_mutex_t OriginalMutex;//������
#endif
	
//�Խ�����װ���Ƽ�ʹ���Խ��������������������쳣�����ǽ����Ȳ���ȫ������������
class Mutex;
class AutoLock
{
public:
	AutoLock( Mutex *pLock );
	~AutoLock();
	void Unlock();
	
private:
	Mutex *m_pMutex;
	bool m_bLocked;
};

//������������װ
class Mutex
{
private:
	//������ϵͳ����
	OriginalMutex m_mutex;
public:
	Mutex();
	~Mutex();
	//����
	void Lock();
	//����
	void Unlock();		
};

}//namespace mdk

#endif//MDK_LOCK_H
