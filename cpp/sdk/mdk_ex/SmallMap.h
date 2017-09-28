// Map.h: interface for the Map class.
//
//Զ�̹�ϡ��
//˼·Դ���ݷ����ģʽ��hash�㷨��Զ�˳������ã�ֱ�Ӵ���int��hashֵ��hash��������
//ʵ��hashֵ�ķֲ�ʽ���㣬����Ч�ʿɴﵽ��ͨ��ϣ��20��30��
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

//Ԫ��
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
	//server����
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

	unsigned long NextPower(unsigned long size);//��size�����С��2��n������
	unsigned int DJBHash(const unsigned char *buf, int len);//C33�㷨hashת������
	bool KeyCmp( unsigned char *key1, int size1, unsigned char *key2, int size2 );//��ͬ����true
	bool Expand(unsigned long size);
	ELEMENT* Find(unsigned char *key, unsigned int size, unsigned int hashValue, bool bInsert );
	void ReleaseHashTable();
	//��ϡ�㷨����ָ��
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
