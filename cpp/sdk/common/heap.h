#ifndef HEAP_H
#define HEAP_H

#include <string>
#include <map>
#include "common/Grid.h"

namespace heap
{
void* Alloc(int size);//������ͨ�ڴ�
void Free(void* buf);//�ͷ���ͨ�ڴ�
bigint Size();//ռ���ڴ��С
}

#endif //HEAP_H
