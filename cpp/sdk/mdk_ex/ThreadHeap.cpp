#include "ThreadHeap.h"
#include <cstdio>

namespace mdk
{

ThreadHeap::ThreadHeap(int objectSize, int count)
{
	m_objectSize = objectSize;
	m_count = count;
	m_pHeader = NULL;
	m_pTail = NULL;
	m_memorySize = 0;
	m_freeCount = 0;
	Expand();
}

ThreadHeap::~ThreadHeap()
{

}

void ThreadHeap::Free(void* pointer)
{
	char *buf = (char*)pointer;
	buf -= sizeof(ThreadHeap::MEMORY);
	ThreadHeap::MEMORY *pMemory = (ThreadHeap::MEMORY*)buf;
	pMemory->pPool->FreeMemory(pMemory);
	return;
}

void* ThreadHeap::Alloc()
{
	if ( !Expand() ) return NULL;

	char *memory = (char*)m_pHeader;
	m_pHeader = m_pHeader->next;
	m_freeCount--;

	return &memory[sizeof(MEMORY)];
}

void ThreadHeap::FreeMemory(MEMORY* pMemory)
{
	if ( NULL == m_pHeader ) 
	{
		m_pHeader = m_pTail = pMemory;
		pMemory->previous = pMemory->next = NULL;
		m_freeCount++;
		return;
	}
	m_pTail->next = pMemory;
	m_pTail = m_pTail->next;
	m_pTail->next = NULL;
	m_freeCount++;

	return;
}

mdk::int64 ThreadHeap::MemorySize()
{
	return m_memorySize;
}

bool ThreadHeap::Expand()
{
	if ( NULL != m_pHeader ) return true;

	char *buf = new char[(sizeof(MEMORY) + m_objectSize) * m_count];
	if ( NULL == buf ) return false;
	m_memorySize += (sizeof(MEMORY) + m_objectSize) * m_count;

	int pos = 0;
	m_pHeader = (MEMORY*)buf;
	m_pTail = m_pHeader;
	MEMORY *pPrevious = NULL;
	MEMORY *pMemory = m_pHeader;
	int i = 0;
	for ( i = 0; i < m_count - 1; i++ )
	{
		pMemory->pPool = this;
		pMemory->previous = pPrevious;
		pos += sizeof(MEMORY) + m_objectSize;
		pMemory->next = (MEMORY*)&buf[pos];
		pPrevious = pMemory;
		pMemory = pMemory->next;
	}
	pMemory->pPool = this;
	pMemory->previous = pPrevious;
	pMemory->next = NULL;
	m_pTail = pMemory;
	m_freeCount = m_count;
	return true;
}

}
