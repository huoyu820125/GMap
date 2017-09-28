// FinishedTime.h: interface for the FinishedTime class.
//
//////////////////////////////////////////////////////////////////////
/*
	ʱ��ͳ����
	��������ͳ��
	����
		int g_t = 0;
		void* CountTime( FinishedTime *pTime )
		{
			mdk::uint32 ut = pTime->UseTime();
			if ( ut > 1000 ) g_t++;
			printf( "XXX����ִ��ʱ�䳬��1000ms�Ĵ���Ϊ%d", gt );
		}
		void XXX()
		{
			FinishedTime count( CountTime );//ͳ��XXX()ִ��ʱ�䣬������������return�ĵط�ȥдͳ�ƴ���
			if ( ����1 ) 
			{
			ִ��...
			return;
			}
			if ( ����... ) 
			{
				ִ��...
				return;
			}
			if ( ����n ) 
			{
				ִ��...
				return;
			}
			ִ��...
		}
 */
#ifndef MDK_FINISHEDTIME_H
#define MDK_FINISHEDTIME_H

#include "Task.h"
#include<ctime>

namespace mdk
{

class FinishedTime
{
public:
	/*
		����ͳ�ƺ���
	*/
	FinishedTime( MethodPointer method, void *pObj );
	FinishedTime( FuntionPointer fun );
	~FinishedTime();

	//������ʱ��ms
	mdk::uint32 UseTime();
	//ִ�а󶨵�ͳ�ƣ�������ǰͳ��
	void Finished();
protected:
private:
	mdk::int64 m_start;
	mdk::int64 m_end;
	mdk::uint32 m_useTime;
	mdk::Task m_task;
	bool m_finished;
};

}

#endif //MDK_FINISHEDTIME_H
