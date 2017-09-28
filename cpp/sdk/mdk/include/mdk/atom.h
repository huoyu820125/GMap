#ifndef MDK_ATOM_H
#define MDK_ATOM_H

#include "FixLengthInt.h"

#ifdef WIN32
//Ϊ�˲�include <windows.h>
#endif

namespace mdk
{

//����,������ֵ
uint32 AtomSelfAdd(void * var); 

//�Լ�,������ֵ
uint32 AtomSelfDec(void * var);

//��һ��ֵ,���ؾ�ֵ
uint32 AtomAdd(void * var, const uint32 value);

//��һ��ֵ,���ؾ�ֵ
uint32 AtomDec(void * var, int32 value);

//��ֵ,windows�·�����ֵ��linux�·��ؾ�ֵ
uint32 AtomSet(void * var, const uint32 value);

//ȡֵ
uint32 AtomGet(void * var);

} //namespace mdk

#endif //MDK_ATOM_H
