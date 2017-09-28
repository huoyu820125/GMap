#ifndef LINK_H
#define LINK_H

namespace mdk
{

//˫������,����ֻ���ڸ����ܵ���ɾ�������ڲ���
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
	void release();//�ͷ��ڴ�
	void clear();//������ݣ����ͷ��ڴ�
	int size();
	Link::NODE* insert( void *data );//�����ݲ�������ĩβ
	void erase(Link::NODE *pNode);//ɾ���ڵ�
	//////////////////////////////////////////////////////////////////////////
	//����
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
	Link::iterator node(Link::NODE *pNode);//�õ�ָ��ڵ�ĵ�����
	void erase(Link::NODE *pNode, Link::iterator &it);//ɾ���ڵ㣬�����µ�����
	void erase(Link::iterator &it);//ɾ��������ָ��Ľڵ㣬������ָ����һ���ڵ�

	//����������
	//���öѷ�������������ڴ��������
	//n��Ԫ����Ҫn+1��Link::NODE
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

