#ifndef THREADHEAP_H
#define THREADHEAP_H

#include "mdk/include/mdk/FixLengthInt.h"

namespace mdk
{

//线程堆内存(非线程安全)
class ThreadHeap
{
public:
	ThreadHeap(int objectSize, int count);
	virtual ~ThreadHeap();

	typedef struct MEMORY
	{
		ThreadHeap *pPool;
		MEMORY *previous;
		MEMORY *next;
	}MEMORY;
	static void Free(void* pointer);
	void* Alloc();
	void FreeMemory(MEMORY* pMemory);
	mdk::int64 MemorySize();

private:
	bool Expand();

	int m_objectSize;
	int m_count;
	MEMORY *m_pHeader;
	MEMORY *m_pTail;
	mdk::int64	m_memorySize;
	int m_freeCount;
};

}

#endif //THREADHEAP_H