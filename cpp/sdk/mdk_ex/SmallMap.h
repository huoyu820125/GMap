// Map.h: interface for the Map class.
//
//远程哈稀表
//思路源于瘦服务端模式，hash算法由远端程序计算好，直接传递int型hash值到hash服务器，
//实现hash值的分布式计算，访问效率可达到普通哈希表20～30倍
//
//////////////////////////////////////////////////////////////////////

#ifndef SMALL_MAP_H
#define SMALL_MAP_H

#ifndef NULL
#define NULL 0
#endif

#ifdef WIN32
#include <windows.h>
#else
#endif
#define MAX_HASH_64 0x7fffffffffffffff
#define MAX_HASH_32 0x7fffffff
typedef	void (*pHashFunction)(unsigned char *hashKey, unsigned int &hashSize, unsigned char *key, unsigned int size );

#include "mdk/include/mdk/FixLengthInt.h"

class SmallMap  
{
public:

//元素
typedef struct ELEMENT
{
	unsigned int hashValue;
	unsigned char *key;
	unsigned short keySize;
	void *value;

	bool isIdle;
	ELEMENT *next;
}ELEMENT;

public:
	SmallMap();
	SmallMap( unsigned long size );
	virtual ~SmallMap();
	

	//////////////////////////////////////////////////////////////////////////
	//server方法
	SmallMap::ELEMENT* Insert(unsigned char *key, unsigned int size, void *value, unsigned int hashValue = 0);
	void* Find(unsigned char *key, unsigned int size, unsigned int hashValue = 0 );
	void* Update(unsigned char *key, unsigned int size, void *value, unsigned int hashValue = 0);
	void* Delete(unsigned char *key, unsigned int size, unsigned int hashValue = 0);
	unsigned long Count();
	bool IsEmpty();
	void Clear();

private:
	unsigned int RemoteHash(unsigned char *key, unsigned int size);
	unsigned int RemoteHash( mdk::uint64 key );

	unsigned long NextPower(unsigned long size);//比size大的最小的2的n次幂数
	unsigned int DJBHash(const unsigned char *buf, int len);//C33算法hash转换函数
	bool KeyCmp( unsigned char *key1, int size1, unsigned char *key2, int size2 );//相同返回true
	bool Expand(unsigned long size);
	ELEMENT* Find(unsigned char *key, unsigned int size, unsigned int hashValue, bool bInsert );
	void ReleaseHashTable();
	//哈稀算法函数指针
	void (*HashFunction)(unsigned char *hashKey, unsigned int &hashSize, 
		unsigned char *key, unsigned int size );		

private:
	bool m_onBit64;
	unsigned long m_maxHash;
	bool m_bRemote;

	ELEMENT	*m_list;
	int		m_space;
	int		m_size;
};

#endif // SMALL_MAP_H
