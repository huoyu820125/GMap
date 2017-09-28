#include "Tree.h"
#include <cstring>
#include <cstdlib>

namespace mdk
{

void* Tree::AllocMethod(int size)
{
	return ::malloc(size);
}

void Tree::FreeMethod(void* pVoid)
{
	::free(pVoid);
}

Tree::Tree()
{
	m_tree = NULL;
	m_size = 0;
	setHeapFun(&Tree::AllocMethod, &Tree::FreeMethod);
}

Tree::~Tree()
{
}

void Tree::clear()
{
	m_tree = NULL;
	m_size = 0;
}

int Tree::size()
{
	return m_size;
}

void Tree::release(Tree::TREE *pTree)
{
	if ( NULL == pTree ) return;

	if ( NULL != pTree->left )
	{
		release(pTree->left);
		pTree->left = NULL;
	}
	if ( NULL != pTree->right )
	{
		release(pTree->right);
		pTree->right = NULL;
	}
	if ( pTree->ownerElement )
	{
		free(pTree->e);
	}
	free(pTree);

	return;
}

void Tree::release()
{
	release(m_tree);
	m_tree = NULL;
	m_size = 0;
}

int Tree::keycmp( char *left, short leftSize, unsigned int leftHash, char *right, short rightSize, unsigned int rightHash )
{
	if ( leftSize != rightSize ) return leftSize - rightSize;
	if ( leftHash != rightHash ) return leftHash - rightHash;

	int i = 0;
	for ( i = 0; i < leftSize; i++ )
	{
		if ( left[i] != right[i] ) return left[i] - right[i];
	}

	return 0;
}

void* Tree::find( void *key, short keySize )
{
	return find((char*)key, keySize, hash(key, keySize));
}

void* Tree::find( char *key, short keySize, unsigned int hashKey )
{
	Tree::TREE *pTree = findNode(key, keySize, hashKey);
	if ( NULL == pTree || pTree->isDeleted ) return NULL;
	return pTree->e->value;
}

Tree::TREE* Tree::findNode( char *key, short keySize, unsigned int hashKey )
{
	TREE *pCur = m_tree;
	int cmp;
	while ( NULL != pCur )
	{
		cmp = keycmp(key, keySize, hashKey, pCur->e->key.data, pCur->e->key.size, pCur->e->key.hashKey);
		if ( 0 == cmp ) 
		{
			if ( pCur->isDeleted ) return NULL;
			return pCur;
		}

		if ( 0 > cmp )
		{
			if ( NULL == pCur->left ) return NULL;
			pCur = pCur->left;
		}
		else
		{
			if ( NULL == pCur->right ) return NULL;
			pCur = pCur->right;
		}
	}

	return NULL;
}

void* Tree::insert( void *key, short keySize, void *data )
{
	return insert((char*)key, keySize, hash(key, keySize), data);
}

void* Tree::insert( char *key, short keySize, unsigned int hashKey, void *data )
{
	TREE *pCur = m_tree;
	int cmp;
	while ( true )
	{
		if ( NULL == pCur )
		{
			pCur = (TREE*)alloc(sizeof(TREE));
			if ( NULL == m_tree ) m_tree = pCur;
			break;
		}

		cmp = keycmp(key, keySize, hashKey, pCur->e->key.data, pCur->e->key.size, pCur->e->key.hashKey);
		if ( 0 == cmp ) 
		{
			if ( !pCur->isDeleted ) return data;
			pCur->isDeleted = false;
			pCur->e->value = data;
			return NULL;
		}

		if ( 0 > cmp )
		{
			if ( NULL == pCur->left ) 
			{
				pCur->left = (TREE*)alloc(sizeof(TREE));
				pCur = pCur->left;
				break;
			}
			pCur = pCur->left;
		}
		else
		{
			if ( NULL == pCur->right )
			{
				pCur->right = (TREE*)alloc(sizeof(TREE));
				pCur = pCur->right;
				break;
			}
			pCur = pCur->right;
		}
	}
	pCur->ownerElement = true;
	pCur->e = (ELEMENT*)alloc(sizeof(ELEMENT));
	pCur->e->key.hashKey = hashKey;
	memcpy(pCur->e->key.data, key, keySize);
	pCur->e->key.size = keySize;
	pCur->e->key.data[keySize] = 0;
	pCur->e->value = data;
	pCur->e->index = 0;

	pCur->isDeleted = false;
	pCur->left = NULL;
	pCur->right = NULL;


	m_size++;
	return NULL;
}

Tree::ELEMENT* Tree::insert( Tree::ELEMENT *e )
{
	TREE *pCur = m_tree;
	int cmp;
	while ( true )
	{
		if ( NULL == pCur )
		{
			pCur = (TREE*)alloc(sizeof(TREE));
			if ( NULL == m_tree ) m_tree = pCur;
			break;
		}
		if ( e == pCur->e )//将数据标记为未删除
		{
			pCur->isDeleted = false;
			return NULL;
		}

		cmp = keycmp(e->key.data, e->key.size, e->key.hashKey, pCur->e->key.data, pCur->e->key.size, pCur->e->key.hashKey);
		if ( 0 == cmp ) 
		{
			if ( pCur->e->index > e->index ) //放弃要插入的数据
			{
				return e;
			}
			if ( pCur->isDeleted ) //数据已删除
			{
				pCur->isDeleted = false;
				m_size++;
			}
			Tree::ELEMENT *pData = pCur->e;
			pCur->e = e;
			return pData;
		}

		if ( 0 > cmp )
		{
			if ( NULL == pCur->left ) 
			{
				pCur->left = (TREE*)alloc(sizeof(TREE));
				pCur = pCur->left;
				break;
			}
			pCur = pCur->left;
		}
		else
		{
			if ( NULL == pCur->right )
			{
				pCur->right = (TREE*)alloc(sizeof(TREE));
				pCur = pCur->right;
				break;
			}
			pCur = pCur->right;
		}
	}
	pCur->ownerElement = false;
	pCur->e = e;
	pCur->isDeleted = false;
	pCur->left = NULL;
	pCur->right = NULL;
	m_size++;

	return NULL;
}

unsigned int Tree::hash( void *key, short keySize )
{
	int i = 0;
	unsigned int hashValue = 0;
	unsigned int val = 0;
	int size = 0;
	unsigned char *buf = (unsigned char*)key;
	for ( ; i < keySize; i += 4 )
	{
		size = i + 3 < keySize?4:keySize - i;
		memcpy(&val, &buf[i], size);
		hashValue += val;
	}
	return hashValue;


// 	unsigned char *buf = (unsigned char*)key;
// 	int i = 0;
// 	int hashValue = 0;
// 	for ( i = 0; i < keySize / 2; i++ ) 
// 	{
// 		hashValue += buf[i];
// 	}
// 
// 	int header = 0;
// 	for ( ; i < keySize; i++ ) 
// 	{
// 		header += buf[i];
// 	}
// 	header = header << 24;
// 	header = header&0x7fffffff;
// 	hashValue += header;
// 
// 	return hashValue;
}

//删除数据返回被删除的数据
void* Tree::erase( void *key, short keySize )
{
	return erase( key, keySize, hash(key, keySize) );
}

void* Tree::erase( void *key, short keySize, unsigned int hashKey )
{
	Tree::TREE *pTree = findNode((char*)key, keySize, hashKey);
	if ( NULL == pTree ) return NULL;
	pTree->isDeleted = true;
	void *pValue = pTree->e->value;
	pTree->e->value = NULL;
	if ( pTree->ownerElement )
	{
		free(pTree->e);
	}
	return pValue;
}

void Tree::setHeapFun( void* (*allocFun)(int size), void (*freeFun)(void* pVoid) )
{
	if ( NULL == allocFun || NULL == freeFun ) return;

	alloc = allocFun;
	free = freeFun;
}

}