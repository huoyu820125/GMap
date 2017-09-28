#ifndef ARRAY_H
#define ARRAY_H

#include <cstdlib>

namespace mdk
{

//数组
template <class T>
class Array
{
public:
	Array()
	{
		setHeapFun(&Array<T>::AllocMethod, &Array<T>::FreeMethod);
		datas = NULL;
		size = 0;
		m_space = 0;
	}

	~Array()
	{
	}

	//释放内存
	void release()
	{
		if ( NULL !=  datas )
		{
			free(datas);
		}
		size = 0;
	}

	//扩容
	void expand()
	{
		int space = m_space * 8;
		if ( 0 >= space ) space = 10240;
		T *newArray = (T*)alloc(sizeof(T) * space);
		int i = 0;
		for ( ; i < size; i++ ) newArray[i] = datas[i];
		if ( NULL != datas ) free(datas);
		datas = newArray;
		m_space = space;

		return;
	}

	void clear()
	{
		size = 0;
	}

	void insert( T data )
	{
		if ( size == m_space ) expand();
		datas[size++] = data;
	}

	void erase(int pos)//删除
	{
		if ( pos >= size ) return;
		for ( ; pos < size -1; pos++ ) datas[pos] = datas[pos+1];
		size--;

		return;
	}

	//高性能设置
	//设置堆方法，用于提高内存分配性能
	//n个元素需要n+1个Array::NODE
	void setHeapFun( void* (*allocFun)(int size), void (*freeFun)(void* pVoid) )
	{
		if ( NULL == allocFun || NULL == freeFun ) return;

		alloc = allocFun;
		free = freeFun;
	}

private:
	static void* AllocMethod(int size)
	{
		return ::malloc(size);
	}

	static void FreeMethod(void* pVoid)
	{
		::free(pVoid);
	}


public:
	T		*datas;
	int		size;

private:
	int		m_space;

	void* (*alloc)(int size);
	void (*free)(void* pVoid);
};

}

#endif //ARRAY_H

