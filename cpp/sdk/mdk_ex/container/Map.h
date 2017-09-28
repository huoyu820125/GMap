#ifndef MAP_H
#define MAP_H

#include "Tree.h"
#include "Link.h"

namespace mdk
{


//占用内存更小，查询熟读更快
class Map
{
private:
	typedef struct ELEMENT
	{
		Tree::ELEMENT data;
		Link::NODE	*pLink;
	}ELEMENT;


public:
	Map();
	virtual ~Map();

	void release();//释放内存
	void clear();//清空数据，不释放内存
	int size();
	void insert( void *key, short keySize, void *data );//key重复时覆盖旧值，旧指针内存将被释放
	void insert( char *key, short keySize, unsigned int hashKey, void *data );//高性能接口
	//删除数据返回被删除的数据
	void* erase( void *key, short keySize );
	void* erase( void *key, short keySize, unsigned int hashKey );//高性能接口

	friend class iterator;
	class iterator
	{
		friend class Map;
	public:
		iterator();
		~iterator();
		iterator& operator++();
		iterator& operator++(int);
		bool isEnd();
		Tree::KEY* key();
		void* data();

	private:
		Link::iterator		m_it;
	};
	Map::iterator find( void *key, short keySize );
	Map::iterator find( char *key, short keySize, unsigned int hashKey );//高性能接口
	Map::iterator begin();

	//高性能设置
	//设置堆方法，用于提高内存分配性能
	//n个元素需要n个Map::ELEMENT、n个Tree::TREE、n+1个Link::NODE
	void setHeapFun( void* (*allocFun)(int size), void (*freeFun)(void* pVoid) );
	//设置数据释放方法
	void setDataFreeFun( void (*freeFun)(void* pVoid) );

	//释放所有数据指针
	void freeAllData();

	//=运算符重载
	Map& operator = (Map &right);
	static unsigned int hash( void *key, short keySize );//hash算法
private:
	void insertR();//真的执行插入
	static void* AllocMethod(int size);
	static void FreeMethod(void* pVoid);

private:
	/*
		数据被检索前只存储在链表中
		检索/遍历操作发生时才被存入m_searcher
	*/
	Link			m_container;	//使用链表存储，用于遍历
	Tree			m_searcher;		//使用搜索树存储，用于检索
	Link::iterator	m_unsearcher;	//指向第一个未被存入m_searcher的数据
	int				m_insertIndex;	//插入顺序号
	int				m_unsearchSize; //未真是插入的数据个数

	void* (*alloc)(int size);
	void (*free)(void* pVoid);
	void (*freeData)(void* pVoid);
};

}
#endif //MAP_H
