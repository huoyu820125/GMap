#include "ShareObject.h"
#include "mdk/include/mdk/atom.h"
#include <cstdio>

namespace mdk
{

ShareObject::ShareObject()
{
	m_refCount = 1;
}

ShareObject::~ShareObject()
{

}

void ShareObject::AddRef()
{
	if ( NULL == this ) return;
	mdk::AtomAdd(&m_refCount, 1);
}

void ShareObject::Release()
{
	if ( NULL == this ) return;
	if ( 1 != mdk::AtomDec(&m_refCount, 1) ) 
	{
		return;
	}
	Delete();
}

void ShareObject::Delete()
{
	delete this;
}

}
