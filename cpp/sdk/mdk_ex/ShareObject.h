#ifndef SHAREOBJECT_H
#define SHAREOBJECT_H

#include "mdk/include/mdk/FixLengthInt.h"

namespace mdk
{
	class ShareObject
	{
	public:
		ShareObject();
		virtual ~ShareObject();

		void AddRef();
		void Release();

	protected:
		virtual void Delete();

	private:
		uint32 m_refCount;
	};
}

#endif //SHAREOBJECT_H
