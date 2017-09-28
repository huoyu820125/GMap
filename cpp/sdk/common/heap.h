#ifndef HEAP_H
#define HEAP_H

#include <string>
#include <map>
#include "common/Grid.h"

namespace heap
{
void* Alloc(int size);//分配普通内存
void Free(void* buf);//释放普通内存
bigint Size();//占用内存大小
}

#endif //HEAP_H
