#ifndef MAP_H
#define MAP_H

#include "Tree.h"
#include "Link.h"

namespace mdk
{


//ռ���ڴ��С����ѯ�������
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

	void release();//�ͷ��ڴ�
	void clear();//������ݣ����ͷ��ڴ�
	int size();
	void insert( void *key, short keySize, void *data );//key�ظ�ʱ���Ǿ�ֵ����ָ���ڴ潫���ͷ�
	void insert( char *key, short keySize, unsigned int hashKey, void *data );//�����ܽӿ�
	//ɾ�����ݷ��ر�ɾ��������
	void* erase( void *key, short keySize );
	void* erase( void *key, short keySize, unsigned int hashKey );//�����ܽӿ�

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
	Map::iterator find( char *key, short keySize, unsigned int hashKey );//�����ܽӿ�
	Map::iterator begin();

	//����������
	//���öѷ�������������ڴ��������
	//n��Ԫ����Ҫn��Map::ELEMENT��n��Tree::TREE��n+1��Link::NODE
	void setHeapFun( void* (*allocFun)(int size), void (*freeFun)(void* pVoid) );
	//���������ͷŷ���
	void setDataFreeFun( void (*freeFun)(void* pVoid) );

	//�ͷ���������ָ��
	void freeAllData();

	//=���������
	Map& operator = (Map &right);
	static unsigned int hash( void *key, short keySize );//hash�㷨
private:
	void insertR();//���ִ�в���
	static void* AllocMethod(int size);
	static void FreeMethod(void* pVoid);

private:
	/*
		���ݱ�����ǰֻ�洢��������
		����/������������ʱ�ű�����m_searcher
	*/
	Link			m_container;	//ʹ������洢�����ڱ���
	Tree			m_searcher;		//ʹ���������洢�����ڼ���
	Link::iterator	m_unsearcher;	//ָ���һ��δ������m_searcher������
	int				m_insertIndex;	//����˳���
	int				m_unsearchSize; //δ���ǲ�������ݸ���

	void* (*alloc)(int size);
	void (*free)(void* pVoid);
	void (*freeData)(void* pVoid);
};

}
#endif //MAP_H
