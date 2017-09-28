#include "OwnLines.h"
#include "common/heap.h"

namespace Grid
{

OwnLines::OwnLines(void)
{
	m_list = NULL;//id����
	m_space = 0;//����
	m_count = 0;//����
}

OwnLines::~OwnLines(void)
{
}

void OwnLines::Release()
{
	if ( NULL != m_list )
	{
		heap::Free(m_list);
		m_list = NULL;
	}
}

void OwnLines::Add(bigint id)
{
	//��ӱ�
	if ( m_count == m_space )
	{
		//����
		int space = m_space * 4;
		if ( 0 == space ) space = 8;
		ELEMENT *newList = (ELEMENT*)heap::Alloc(sizeof(ELEMENT)*space);
		//����Ǩ��
		int i = 0;
		for ( ; i < m_count; i++ ) newList[i] = m_list[i];

		if ( NULL != m_list ) 
		{
			heap::Free(m_list);
			m_list = NULL;
		}
		m_space = space;
		m_list = newList;
	}

	//��ӱ�
	m_list[m_count].id = id;
	m_list[m_count].pointer = NULL;
	m_count++;

	return;
}

bool OwnLines::Del(bigint id)
{
	int i = 0;
	for ( ; i < m_count; i++ )
	{
		if ( id == m_list[i].id ) 
		{
			m_count--;
			for ( ; i < m_count; i++ ) m_list[i] = m_list[i+1];
			return true;
		}
	}

	return false;
}

bool OwnLines::DelByIndex(int i)
{
	if ( i >= m_count ) return false;
	m_count--;
	for ( ; i < m_count; i++ ) m_list[i] = m_list[i+1];

	return true;
}

}
