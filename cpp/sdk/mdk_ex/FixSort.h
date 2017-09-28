#ifndef FIXSORT_H
#define FIXSORT_H

namespace mdk
{

/*
	修正排序
	当容器中某个对象改变时，其它未改变的对象相对顺序是保持不变的，
	因此只需要将被改变的对象的位置进行修正（移动），就可以实现完全重排序，
	而不必对全部数据重新排序，大大提升在排序对象的值经常变化时的重排性能。
	
	尤其适用于排名这类场景，对象的值总是在变，但每次改变都需要正确的顺序。
*/
template<class T>
class FixSort
{
	//return < 0 pObj1在pObj2前（升序序排列），否则pObj1在pObj2后（降序排列）
	typedef int (*CmpFuntion)(const T &pObj1, const T &pObj2);
	//双向链表结点
	typedef struct Item
	{
		T *buffer;//保存对象地址
		T object;//排序好的旧值，用来和buffer中的真实值比较重新排序
		Item *previous;
		Item *next;
	}Item;
public:
	//索引
	class Index
	{
	public:
		Index( FixSort &container, bool fromHeader = true )
			:m_container(container)
		{ 
			m_fromHeader = fromHeader; 
		}
		~Index(){}
		void Begin()
		{
			if ( m_fromHeader ) m_cur = m_container.m_header;
			else m_cur = m_container.m_tail;

			return;
		}

		bool IsEnd()
		{
			if ( NULL == m_cur ) return true;
			return false;
		}

		Index& operator ++()
		{
			if ( m_fromHeader ) m_cur = m_cur->next;
			else m_cur = m_cur->previous;
			return *this;
		}

		operator T*()
		{
			return m_cur->buffer;
		}

	private:
		FixSort &m_container;
		Item *m_cur;
		bool m_fromHeader;
	};
public:
	/*
		cmp：告诉链表如何比较2个对象的大小
	*/
	FixSort( CmpFuntion cmp )
	{
		m_count = 0;
		m_header = m_tail = NULL;
		m_cmp = cmp;
	}

	~FixSort()
	{

	}

	//插入对象，并排序
	bool Insert( T *pObj )
	{
		if ( NULL == m_cmp ) 
		{
			return false;
		}
		if ( NULL == pObj ) 
		{
			return false;
		}

		if ( m_itemMap.end() != m_itemMap.find( pObj )  ) 
		{
			Change(pObj);
			return true;
		}

		m_count++;
		Item *last = NULL;
		Item *cur = m_header;
		for ( cur = m_header; NULL != cur; cur = cur->next )
		{
			if ( m_cmp( cur->object, *pObj ) < 0 )//找到插入位置
			{
				break;
			}
			last = cur;
		}
		Item *pItem = new Item;
		pItem->object = *pObj;
		pItem->buffer = pObj;
		pItem->previous = last;
		pItem->next = cur;
		if ( NULL == cur ) m_tail = pItem;
		m_itemMap[pObj] = pItem;

		if ( NULL == last )
		{
			pItem->next = m_header;
			if ( NULL != m_header ) m_header->previous = pItem;
			m_header = pItem;
			return true;
		}
		last->next = pItem;
		if ( NULL != cur ) cur->previous = pItem;

		return true;
	}

	//某1个对象改变了，对这个对象重排序
	void Change( T *pObj )
	{
		Item *change = NULL;
		typename std::map<void*, Item* >::iterator it;
		it = m_itemMap.find( pObj );
		if ( it == m_itemMap.end() ) return;
		change = it->second;

		int changeType = m_cmp( *pObj, change->object );
		change->object = *pObj;

		if ( 0 == changeType ) return;

		if ( 0 < changeType )//名次可能上升
		{
			bool toHead = true;
			Item *previous = NULL;
			for ( previous = change->previous; previous; previous = previous->previous )
			{
				if ( m_cmp( change->object, previous->object ) > 0 ) 
				{
					if ( NULL == previous->previous ) break;
					continue;
				}
				toHead = false;
				break;
			}
			if ( NULL == previous ) return;
			if ( previous->next == change && !toHead ) return;

			//将change从列表中摘出来
			change->previous->next = change->next;
			if ( NULL != change->next ) change->next->previous = change->previous;

			if ( change == m_tail ) m_tail = change->previous;

			if ( toHead )
			{
				change->next = previous;
				change->previous = previous->previous;
				previous->previous = change;
				m_header = change;
				return;
			}

			//将change插入到previous之后
			change->next = previous->next;
			change->previous = previous;

			//将previous的下一个对象接到change之后
			previous->next->previous = change;
			previous->next = change;
			return;
		}

		//名次可能下降
		bool toTail = true;
		Item *next = NULL;
		for ( next = change->next; next; next = next->next )
		{
			if ( m_cmp( change->object, next->object ) < 0 )
			{
				if ( NULL == next->next ) break;
				continue;
			}
			toTail = false;
			break;
		}
		if ( NULL == next ) return;
		if ( next->previous == change && !toTail ) return;

		//将change从列表中摘出来
		if ( NULL != change->previous ) change->previous->next = change->next;
		change->next->previous = change->previous;

		if ( m_header == change ) m_header = change->next;

		if ( toTail )
		{
			change->next = next->next;
			change->previous = next;
			next->next = change;
			return;
		}

		//将change插入到next之前
		change->next = next;
		change->previous = next->previous;
		next->previous->next = change;
		next->previous = change;

		return;
	}

	int Size()
	{
		return m_count;
	}

	T* operator[](int pos)
	{
		if ( pos >= m_count )
		{
			char *p = NULL;
			*p = 1;
			return NULL;
		}

		int i = 0;
		Item *pItem = m_header;
		for ( i = 0; i < pos; i++ )
		{
			pItem = pItem->next;
		}

		return pItem->buffer;
	}
	//检查排序结果是否正确
	int Check()
	{
		int i = 0; 
		Item *cur = m_header;
		for ( i = 0; i < m_count - 1; i++ )
		{
			if ( NULL == cur->next )//表头错误
			{
				return 1;
			}
			if ( m_cmp( cur->object, cur->next->object ) < 0 )//算法有误
			{
				return 2;
			}
			if ( cur != cur->next->previous )//链表有误
			{
				return 3;
			}
			cur = cur->next;
		}

		return 0;
	}

private:
	Item *m_header;
	Item *m_tail;
	int	m_count;
	CmpFuntion m_cmp;
	std::map<void*, Item*> m_itemMap;
};

}

#endif //FIXSORT_H
