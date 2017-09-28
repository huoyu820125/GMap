#ifndef FIXSORT_H
#define FIXSORT_H

namespace mdk
{

/*
	��������
	��������ĳ������ı�ʱ������δ�ı�Ķ������˳���Ǳ��ֲ���ģ�
	���ֻ��Ҫ�����ı�Ķ����λ�ý����������ƶ������Ϳ���ʵ����ȫ������
	�����ض�ȫ�������������򣬴����������������ֵ�����仯ʱ���������ܡ�
	
	�����������������ೡ���������ֵ�����ڱ䣬��ÿ�θı䶼��Ҫ��ȷ��˳��
*/
template<class T>
class FixSort
{
	//return < 0 pObj1��pObj2ǰ�����������У�������pObj1��pObj2�󣨽������У�
	typedef int (*CmpFuntion)(const T &pObj1, const T &pObj2);
	//˫��������
	typedef struct Item
	{
		T *buffer;//��������ַ
		T object;//����õľ�ֵ��������buffer�е���ʵֵ�Ƚ���������
		Item *previous;
		Item *next;
	}Item;
public:
	//����
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
		cmp������������αȽ�2������Ĵ�С
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

	//������󣬲�����
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
			if ( m_cmp( cur->object, *pObj ) < 0 )//�ҵ�����λ��
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

	//ĳ1������ı��ˣ����������������
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

		if ( 0 < changeType )//���ο�������
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

			//��change���б���ժ����
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

			//��change���뵽previous֮��
			change->next = previous->next;
			change->previous = previous;

			//��previous����һ������ӵ�change֮��
			previous->next->previous = change;
			previous->next = change;
			return;
		}

		//���ο����½�
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

		//��change���б���ժ����
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

		//��change���뵽next֮ǰ
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
	//����������Ƿ���ȷ
	int Check()
	{
		int i = 0; 
		Item *cur = m_header;
		for ( i = 0; i < m_count - 1; i++ )
		{
			if ( NULL == cur->next )//��ͷ����
			{
				return 1;
			}
			if ( m_cmp( cur->object, cur->next->object ) < 0 )//�㷨����
			{
				return 2;
			}
			if ( cur != cur->next->previous )//��������
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
