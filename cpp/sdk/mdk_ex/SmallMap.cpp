// SmallMap.cpp: implementation of the SmallMap class.
//
//////////////////////////////////////////////////////////////////////

#include "SmallMap.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include "mdk/include/mdk/mapi.h"
#include "mdk_ex/MD5Helper.h"

static void Md5Hash(unsigned char *hashKey, unsigned int &hashSize, unsigned char *key, unsigned int size )
{
	MD5Helper helper;
	std::string hk = helper.HashString(key, size);
	hashSize = hk.size();
	memcpy(hashKey, hk.c_str(), hashSize);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SmallMap::SmallMap()
{
	m_onBit64 = false;
	int *bit = NULL;
	if ( 8 == sizeof(bit) ) m_onBit64 = true;

#if (m_onBit64)
	if ( m_onBit64 ) m_maxHash = MAX_HASH_64;
#else
	m_maxHash = MAX_HASH_32;
#endif
	HashFunction = Md5Hash;
	m_size = 0;
	Expand(5);
}

SmallMap::SmallMap(unsigned long size)
{
	m_onBit64 = false;
	int *bit = NULL;
	if ( 8 == sizeof(bit) ) m_onBit64 = true;

#if (m_onBit64)
	if ( m_onBit64 ) m_maxHash = MAX_HASH_64;
#else
	m_maxHash = MAX_HASH_32;
#endif
	HashFunction = Md5Hash;
	m_size = 0;
	Expand(size);
}

SmallMap::~SmallMap()
{
	ReleaseHashTable();
}

unsigned long SmallMap::NextPower(unsigned long size)
{
    unsigned long i = 8;
	if (size >= m_maxHash) return m_maxHash;
	while(1) 
	{
        if (i >= size) return i;
        i *= 2;
    }
}

unsigned int SmallMap::DJBHash(const unsigned char *buf, int len) 
{
    unsigned int hash = 5381;

    while (len--)
	{
        hash = ((hash << 5) + hash) + (*buf++); /* hash * 33 + c */
	}

    return hash;
}

bool SmallMap::KeyCmp( unsigned char *key1, int size1, unsigned char *key2, int size2 )
{
	if ( size1 != size2 ) return false;
	int i = 0;
	for ( i = 0; i < size1; i++ )
	{
		if ( key1[i] != key2[i] ) return false;
	}
	
	return true;
}

bool SmallMap::Expand(unsigned long size)
{
	ELEMENT *old = m_list;
	int oldSpace = m_space;
	m_space += size;
	ELEMENT *m_list = new ELEMENT[m_space];
	int i = 0;
	ELEMENT *cur;
	for ( i = 0; i < oldSpace; i++ )
	{
		if ( old[i].isIdle ) continue;
		cur = Find(old[i].key, old[i].keySize, old[i].hashValue, false);
		cur->hashValue = old[i].hashValue;
		cur->key = old[i].key;
		cur->keySize = old[i].keySize;
		cur->value = old[i].value;
		cur->isIdle = old[i].isIdle;
		cur->next;
	}
	delete[]old;

	return true;
}

unsigned int SmallMap::RemoteHash( mdk::uint64 key )
{
	return key;
}

unsigned int SmallMap::RemoteHash( unsigned char *key, unsigned int size )
{
	mdk::mdk_assert(NULL != HashFunction);
	unsigned char hashKey[256];
	unsigned int hashSize;
	HashFunction( hashKey, hashSize, key, size );
	return DJBHash( hashKey, hashSize );
}

SmallMap::ELEMENT* SmallMap::Find(unsigned char* key, unsigned int size, unsigned int hashValue,  bool bInsert )
{
	hashValue = RemoteHash(key, size);
	unsigned int pos = 0;
	int i = 0;

	pos = hashValue&m_space;
	for ( i = 0; i < m_space; i++ )
	{
		if (m_list[pos].isIdle
			|| hashValue != m_list[pos].hashValue 
			|| !KeyCmp( m_list[pos].key, m_list[pos].keySize, key, size ) ) 
		{
			pos++;
			if ( pos == m_space ) pos = 0;
			continue;
		}
		if (bInsert) return NULL;

		return &m_list[pos];
	}

	ELEMENT *pFind = NULL;
	pos = hashValue&m_space;
	for ( i = 0; i < m_space; i++ )
	{
		if (m_list[pos].isIdle) 
		{
			pFind = &m_list[pos];
			break; 
		}

		pos++;
		if ( pos == m_space ) pos = 0;
	}

	if ( bInsert )
	{
		if ( NULL == pFind ) 
		{
			Expand(m_space);
			return Find(key, size, hashValue, bInsert);
		}
		pFind->value = NULL;
		pFind->keySize = size;
		if ( pFind->keySize > 0 )
		{
			pFind->key = new unsigned char[pFind->keySize];
			if ( NULL == pFind->key ) return NULL;
			memcpy( pFind->key, key, pFind->keySize );
		}
		pFind->hashValue = hashValue;
		pFind->isIdle = false;
		m_size++;
	}

	return pFind;
}

void* SmallMap::Find(unsigned char *key, unsigned int size, unsigned int hashValue )
{
	mdk::mdk_assert(m_bRemote||NULL != HashFunction);
	ELEMENT *pFindE = Find( key, size, hashValue, false );
	if ( NULL == pFindE ) return NULL;
	return pFindE->value;
}

SmallMap::ELEMENT* SmallMap::Insert(unsigned char *key, unsigned int size, void *value, unsigned int hashValue)
{
	mdk::mdk_assert(m_bRemote||NULL != HashFunction);
	ELEMENT *pFind = Find( key, size, hashValue, true );
	if ( NULL == pFind ) return NULL;
	pFind->value = value;
	
	return pFind;
}

void* SmallMap::Update(unsigned char *key, unsigned int size, void *value, unsigned int hashValue)
{
	mdk::mdk_assert(m_bRemote||NULL != HashFunction);
	ELEMENT *pFindE = Find( key, size, hashValue, false );
	if ( NULL == pFindE ) return NULL;

	void *old = pFindE->value;
	pFindE->value = value;
	return old;
}

void* SmallMap::Delete(unsigned char *key, unsigned int size, unsigned int hashValue)
{
	mdk::mdk_assert(m_bRemote||NULL != HashFunction);
	ELEMENT *pFindE = Find( key, size, hashValue, false );
	if ( NULL == pFindE ) return NULL;
	pFindE->isIdle = true;
	void *old = pFindE->value;
	pFindE->value = NULL;
	if ( pFindE->keySize > 0 )
	{
		delete pFindE->key;
		pFindE->key = NULL;
		pFindE->keySize = 0;
	}
	m_size--;
	return old;
}

unsigned long SmallMap::Count()
{
	return m_size;
}

bool SmallMap::IsEmpty()
{
	return 0 == Count();
}

void SmallMap::Clear()
{
	int i = 0;
	for ( i = 0; i < m_space; i++ )
	{
		if (m_list[i].isIdle) continue;

		m_list[i].hashValue = 0;
		if ( 0 < m_list[i].keySize )
		{
			delete[]m_list[i].key;
			m_list[i].keySize = 0;
			m_list[i].key = NULL;
		}
		m_list[i].value = NULL;
		m_list[i].isIdle = true;
	}

	return;
}

void SmallMap::ReleaseHashTable()
{
	Clear();
	delete[]m_list;
	m_list = NULL;
}
