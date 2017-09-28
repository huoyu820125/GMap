#include "Link.h"
#include <cstring>

#include <cstdlib>

namespace mdk
{
void* Link::AllocMethod(int size)
{
	return ::malloc(size);
}

void Link::FreeMethod(void* pVoid)
{
	::free(pVoid);
}

Link::Link()
{
	m_header = m_tail = NULL;
	m_size = 0;
	setHeapFun(&Link::AllocMethod, &Link::FreeMethod);
}

Link::~Link()
{
}

void Link::clear()
{
	m_header = m_tail = NULL;
	m_size = 0;
}

int Link::size()
{
	return m_size;
}

void Link::release()
{
	NODE *pCur = m_header;
	NODE *pDel;
	while ( NULL !=  pCur )
	{
		pDel = pCur;
		pCur = pCur->next;
		free(pDel);
	}
	m_header = m_tail = NULL;
	m_size = 0;
}

Link::NODE* Link::insert( void *data )
{
	if ( NULL == m_header )
	{
		m_tail = m_header = (NODE*)alloc(sizeof(NODE));
		m_header->parent = NULL;
		m_header->next = NULL;
	}
	m_tail->data = data;
	m_tail->next = (NODE*)alloc(sizeof(NODE));
	m_tail->next->parent = m_tail;
	m_tail = m_tail->next;
	m_tail->next = NULL;
	m_size++;

	return m_tail->parent;
}

void Link::erase(Link::NODE *pNode)
{
	if ( m_header == pNode )
	{
		m_header = pNode->next;
		m_header->parent = NULL;
		free(pNode);
		m_size--;
		return;
	}
	pNode->parent->next = pNode->next;
	pNode->next->parent = pNode->parent;
	free(pNode);
	m_size--;
	return;
}

void Link::erase(Link::NODE *pNode, Link::iterator &it)
{
	if ( this == it.m_owner && pNode == it.m_pElement ) it++;
	erase(pNode);
	return;
}

void Link::erase(Link::iterator &it)
{
	Link::NODE *pNode = it.m_pElement;
	it++;
	erase(pNode);
	return;
}

Link::iterator Link::begin()
{
	iterator ret;
	ret.m_owner = this;
	ret.m_pElement = m_header;

	return ret;
}

Link::iterator Link::end()
{
	iterator ret;
	ret.m_owner = this;
	ret.m_pElement = m_tail;

	return ret;
}

Link::iterator Link::node(Link::NODE *pNode)
{
	iterator ret;
	ret.m_owner = this;
	ret.m_pElement = pNode;

	return ret;
}


void Link::setHeapFun( void* (*allocFun)(int size), void (*freeFun)(void* pVoid) )
{
	if ( NULL == allocFun || NULL == freeFun ) return;

	alloc = allocFun;
	free = freeFun;
}


Link::iterator::iterator()
{
	m_owner = NULL;
	m_pElement = NULL;
}

Link::iterator::~iterator()
{
}

bool Link::iterator::isInit()
{
	return NULL != m_owner;
}

void* Link::iterator::data()
{
	if ( NULL == m_pElement ) return NULL;

	return m_pElement->data;
}

bool Link::iterator::isHeader()
{
	return m_owner->m_header == m_pElement;
}

bool Link::iterator::isEnd()
{
	return m_owner->m_tail == m_pElement;
}

Link::iterator& Link::iterator::operator++()
{
	if ( m_owner->m_tail == m_pElement ) return *this;
	m_pElement = m_pElement->next;

	return *this;
}

Link::iterator& Link::iterator::operator++(int)
{
	if ( m_owner->m_tail == m_pElement ) return *this;
	m_pElement = m_pElement->next;

	return *this;
}

Link::iterator& Link::iterator::operator--()
{
	if ( m_owner->m_header == m_pElement ) return *this;
	m_pElement = m_pElement->parent;

	return *this;
}

Link::iterator& Link::iterator::operator--(int)
{
	if ( m_owner->m_header == m_pElement ) return *this;
	m_pElement = m_pElement->parent;

	return *this;
}

void Link::iterator::setOwner(Link *owner)
{
	m_owner = owner;
}

}