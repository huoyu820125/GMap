#include "Map.h"
#include <cstring>
#include <vector>
#include <algorithm>

namespace mdk
{

void* Map::AllocMethod(int size)
{
	return ::malloc(size);
}

void Map::FreeMethod(void* pVoid)
{
	::free(pVoid);
}

Map::Map()
{
	m_insertIndex = 0;
	m_unsearchSize = 0;
	setHeapFun(&Map::AllocMethod, &Map::FreeMethod);
	setDataFreeFun(&Map::FreeMethod);
}

Map::~Map()
{
}

void Map::clear()
{
	m_insertIndex = 0;
	m_unsearchSize = 0;
	m_searcher.clear();
	m_container.clear();
	Link::iterator it;
	m_unsearcher = it;
}

int Map::size()
{
	insertR();
	return m_container.size();
}

void Map::release()
{
	m_searcher.release();
	Link::iterator it = m_container.begin();
	for ( ; !it.isEnd(); it++ )
	{
		ELEMENT *p = (ELEMENT*)it.data();
		delete p;
	}
	m_container.release();
	Link::iterator itN;
	m_unsearcher = itN;
}

void Map::freeAllData()
{
	if ( NULL != freeData )
	{
		Map::iterator it = begin();
		for ( ; !it.isEnd(); it++ )
		{
			if ( NULL != it.data() ) freeData(it.data());
		}
	}
}

void Map::insert( void *key, short keySize, void *data )
{
	insert((char*)key, keySize, hash(key, keySize), data);
	return;
}

void Map::insert( char *key, short keySize, unsigned int hashKey, void *data )
{
	ELEMENT *pData = new ELEMENT;

	pData->data.key.hashKey = hashKey;
	memcpy(pData->data.key.data, key, keySize);
	pData->data.key.size = keySize;
	pData->data.key.data[keySize] = 0;
	pData->data.value = data;
	pData->data.index = m_insertIndex++;
	pData->pLink = m_container.insert(pData);
	if ( !m_unsearcher.isInit() )
	{
		m_unsearcher = m_container.begin();
	}
	if ( 1 == m_container.size() ) 
	{
		insertR();
		return;
	}
	m_unsearchSize++;
	if ( 100000 > m_unsearchSize ) return;//没有查询操作,数量小于10万,不执行真是插入
	insertR();//所有数据打乱顺序进入世界

	return;
}

unsigned int Map::hash( void *key, short keySize )
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

void Map::insertR()
{
	if ( 0 == m_container.size() ) return;
	m_unsearchSize = 0;
	m_unsearcher.setOwner(&m_container);
	if ( m_unsearcher.isEnd() ) return;
	//打乱顺序插入二叉搜索树，避免插入数据是序列化的
	std::vector<Tree::ELEMENT*> v;
	for ( ; !m_unsearcher.isEnd(); m_unsearcher++ )
	{
		v.push_back((Tree::ELEMENT*)m_unsearcher.data());
	}
	std::random_shuffle(v.begin(), v.end());

	ELEMENT *pOld;
	int i = 0;
	for ( i = 0; i < v.size(); i++ ) 
	{
		pOld = (ELEMENT*)m_searcher.insert(v[i]);
		if ( NULL == pOld ) continue;
		m_container.erase(pOld->pLink, m_unsearcher);
		if ( NULL != pOld->data.value && NULL != freeData )
		{
			freeData(pOld->data.value);
		}
		delete pOld;
	}
}

//删除数据返回被删除的数据
void* Map::erase( void *key, short keySize )
{
	return erase( key, keySize, hash(key, keySize) );
}

void* Map::erase( void *key, short keySize, unsigned int hashKey )
{
	insertR();

	Tree::TREE *pTree = m_searcher.findNode((char*)key, keySize, hashKey);
	if ( NULL == pTree || pTree->isDeleted ) return NULL;

	void *pValue = pTree->e->value;
	pTree->isDeleted = true;
	pTree->e->value = NULL;
	ELEMENT *pElement = (ELEMENT*)pTree->e;
	m_container.erase(pElement->pLink, m_unsearcher);

	return pValue;
}

Map::iterator Map::find( void *key, short keySize )
{
	return find((char*)key, keySize, hash(key, keySize));
}

Map::iterator Map::find( char *key, short keySize, unsigned int hashKey )
{
	insertR();
	Map::iterator ret;
	ret.m_it = m_container.end();
	Tree::TREE *pTree = m_searcher.findNode(key, keySize, hashKey);
	if ( NULL == pTree || pTree->isDeleted ) return ret;
	ret.m_it = m_container.node(((ELEMENT*)pTree->e)->pLink);
	return ret;
}

Map::iterator Map::begin()
{
	insertR();
	iterator ret;
 	ret.m_it = m_container.begin();

	return ret;
}

Map::iterator::iterator()
{
}

Map::iterator::~iterator()
{
}

void* Map::iterator::data()
{
	if ( m_it.isEnd() ) return NULL;

	ELEMENT *pData = (ELEMENT*)m_it.data();

	return pData->data.value;
}

Tree::KEY* Map::iterator::key()
{
	if ( m_it.isEnd() ) return NULL;

	ELEMENT *pData = (ELEMENT*)m_it.data();

	return &pData->data.key;
}


bool Map::iterator::isEnd()
{
	return m_it.isEnd();
}

Map::iterator& Map::iterator::operator++()
{
	m_it++;
	return *this;
}

Map::iterator& Map::iterator::operator++(int)
{
	m_it++;
	return *this;
}

void Map::setHeapFun( void* (*allocFun)(int size), void (*freeFun)(void* pVoid) )
{
	alloc = allocFun;
	free = freeFun;
	m_container.setHeapFun(allocFun, freeFun);
	m_searcher.setHeapFun(allocFun, freeFun);
}

void Map::setDataFreeFun( void (*freeFun)(void* pVoid) )
{
	freeData = freeFun;
}

Map& Map::operator = (Map &right)
{
	m_container = right.m_container;	//使用链表存储，用于遍历
	m_searcher = right.m_searcher;		//使用搜索树存储，用于检索
	m_unsearcher = right.m_unsearcher;
	if ( 0 < m_container.size() )
	{
		m_unsearcher.setOwner(&m_container);
	}
	m_insertIndex = right.m_insertIndex;	//插入顺序号
	m_unsearchSize = right.m_unsearchSize; //未真是插入的数据个数
	
	return *this;
}

}
