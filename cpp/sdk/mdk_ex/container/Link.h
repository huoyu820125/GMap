#ifndef LINK_H
#define LINK_H

namespace mdk
{

//双向链表,仅仅只用于高性能的增删，不用于查找
class Link
{
public:
	Link();
	virtual ~Link();

	typedef struct NODE
	{
		void	*data;
		NODE	*parent;
		NODE	*next;
	}NODE;
	void release();//释放内存
	void clear();//清空数据，不释放内存
	int size();
	Link::NODE* insert( void *data );//将数据插入链表末尾
	void erase(Link::NODE *pNode);//删除节点
	//////////////////////////////////////////////////////////////////////////
	//遍历
	friend class iterator;
	class iterator
	{
		friend class Link;
	public:
		iterator();
		~iterator();
		iterator& operator++();
		iterator& operator++(int);
		iterator& operator--();
		iterator& operator--(int);
		bool isInit();
		bool isHeader();
		bool isEnd();
		void* data();
		void setOwner(Link *owner);

	public:
		Link	*m_owner;
		NODE	*m_pElement;
	};
	Link::iterator begin();
	Link::iterator end();
	Link::iterator node(Link::NODE *pNode);//得到指向节点的迭代器
	void erase(Link::NODE *pNode, Link::iterator &it);//删除节点，并更新迭代器
	void erase(Link::iterator &it);//删除迭代器指向的节点，迭代器指向下一个节点

	//高性能设置
	//设置堆方法，用于提高内存分配性能
	//n个元素需要n+1个Link::NODE
	void setHeapFun( void* (*allocFun)(int size), void (*freeFun)(void* pVoid) );

private:
	static void* AllocMethod(int size);
	static void FreeMethod(void* pVoid);

public:
	NODE	*m_header;
	NODE	*m_tail;
	int		m_size;

	void* (*alloc)(int size);
	void (*free)(void* pVoid);
};

}

#endif //LINK_H

