#include "heap.h"
#include "common/struct/Point.h"
#include "common/struct/Line.h"
#include "mdk_ex/ThreadHeap.h"

namespace heap
{
mdk::ThreadHeap g_lineHeap(sizeof(bigint) + sizeof(bigint) + sizeof(Grid::Line), 10000);
mdk::ThreadHeap g_pointHeap(sizeof(bigint) + sizeof(bigint) + sizeof(Grid::Point), 10000);
bigint memorySize = 0;//当前使用内存大小

void* Alloc(int size)
{
	return new char[size];
}

void Free(void* buf)
{
	delete[]buf;
}

bigint Size()
{
	return memorySize;
}

}