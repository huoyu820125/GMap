// RHTable.h: interface for the RHTable class.
//
//Զ�̹�ϡ��
//˼·Դ���ݷ����ģʽ��hash�㷨��Զ�˳������ã�ֱ�Ӵ���int��hashֵ��hash��������
//ʵ��hashֵ�ķֲ�ʽ���㣬����Ч�ʿɴﵽ��ͨ��ϣ��20��30��
//
//////////////////////////////////////////////////////////////////////

#ifndef REMOTE_HASHTABLE_H
#define REMOTE_HASHTABLE_H

#ifdef WIN32
#include <windows.h>
#else
#endif
#define MAX_HASH_64 0x7fffffffffffffff
#define MAX_HASH_32 0x7fffffff
typedef	unsigned int (*pHashFunction)(unsigned char *hashKey, unsigned int hashSize);

#include "mdk/include/mdk/FixLengthInt.h"

namespace mdk
{

class RHTable  
{
	struct ELEMENT;
	struct HASH_TABLE;
public:
//�������
typedef struct OP_R
{
	ELEMENT *pInsert;
	bool bSuccess;
}OP_R;

//������
typedef struct Iterator
{
	Iterator& operator++();
	Iterator& operator++(int);
	Iterator& operator--();
	Iterator& operator--(int);
	bool operator==(Iterator it);
	bool operator!=(Iterator it);
	ELEMENT *pElement;
private:
	friend class RHTable;
	unsigned int idx;
	HASH_TABLE *pHashTable;
	HASH_TABLE *pHeadHashTable;
}Iterator;

private:
//Ԫ��
typedef struct ELEMENT
{
	unsigned int hashValue;
	unsigned char *key;
	unsigned short keySize;
	void *value;
private:
	friend class RHTable;
	friend struct Iterator;
	bool isDel;
	ELEMENT *next;//ͬһ��Ͱ����һ��Ԫ��
}ELEMENT;

//Ͱ�����淢��hash��ײ��Ԫ��
typedef struct BUCKET
{
	ELEMENT *head;//Ͱ��Ԫ������ͷ
}BUCKET;

//��ϡ��
typedef struct HASH_TABLE
{
	BUCKET *buckets;//hash����
	unsigned long size;//hash�����С
	unsigned long sizemask;//����size��ȫ1��ʾ
	HASH_TABLE *pre;//ǰһ����
	HASH_TABLE *next;//��һ������hash��
	unsigned long count;//ʵ��Ԫ������
}HASH_TABLE;

public:
	RHTable();
	RHTable( unsigned long size );
	virtual ~RHTable();
	void SetRemoteMode( bool bRemote );
	void SetHashFunction( pHashFunction hf );

	//////////////////////////////////////////////////////////////////////////
	//client����,��������ֵ��key����hash
	unsigned int RemoteHash(unsigned char *key, unsigned int size);
	unsigned int RemoteHash( mdk::uint8 key );
	unsigned int RemoteHash( mdk::uint16 key );
	unsigned int RemoteHash( mdk::uint32 key );
	unsigned int RemoteHash( mdk::uint64 key );
	
	//////////////////////////////////////////////////////////////////////////
	//server����
	OP_R* Insert(unsigned char *key, unsigned int size, void *value, unsigned int hashValue = 0);
	void* Find(unsigned char *key, unsigned int size, unsigned int hashValue = 0 );
	void* Update(unsigned char *key, unsigned int size, void *value, unsigned int hashValue = 0);
	void Delete(unsigned char *key, unsigned int size, unsigned int hashValue = 0);
	unsigned long Count();
	bool IsEmpty();
	void Clear();
	Iterator Begin();
	Iterator End();

private:
	unsigned long NextPower(unsigned long size);//��size�����С��2��n������
	bool KeyCmp( unsigned char *key1, int size1, unsigned char *key2, int size2 );//��ͬ����true
	bool Expand(unsigned long size);
	ELEMENT* Find(unsigned char *key, unsigned int size, unsigned int hashValue, bool bInsert );
	void ReleaseOldHashTable();//��hash�����Ϊnull���ͷ�
	void ReleaseHashTable();
	//��ϡ�㷨����ָ��
	unsigned int (*HashFunction)(unsigned char *hashKey, unsigned int hashSize);		
	
private:
	HASH_TABLE *m_pHashTable;
	bool m_onBit64;
	unsigned long m_maxHash;
	Iterator m_it;
	bool m_bRemote;
};

}

#endif // REMOTE_HASHTABLE_H
