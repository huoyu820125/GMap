#ifndef TREE_H
#define TREE_H

namespace mdk
{

//����������
//����αɾ��
class Tree
{
public:
	Tree();
	virtual ~Tree();

	typedef struct KEY
	{
		char	data[65];
		short	size;
		unsigned int hashKey;
	}KEY;

	typedef struct ELEMENT
	{
		KEY		key;
		void	*value;
		int		index;//����˳��ţ������ظ������ж��¾�
	};

	typedef struct TREE
	{
		ELEMENT	*e;
		bool	isDeleted;
		TREE	*left;
		TREE	*right;
		bool	ownerElement;
	}TREE;

	void release();//�ͷ��ڴ�
	void clear();//������ݣ����ͷ��ڴ�
	int size();
	void* find( void *key, short keySize );
	void* find( char *key, short keySize, unsigned int hashKey );//�����ܽӿ�
	Tree::TREE* findNode( char *key, short keySize, unsigned int hashKey );//�����ܽӿ�
	/*
		data������new
		key�ظ�ʱ�����ر�����������
	*/
	void* insert( void *key, short keySize, void *data );//�ظ������Ѵ��ڵ�Ԫ��
	void* insert( char *key, short keySize, unsigned int hashKey, void *data );//�����ܽӿ�
	/*
		ֱ�Ӳ��봴���õ�Ԫ�أ����ڶ����������ͬһ�������ݵĳ���
		����copy�������ڴ�ռ�ã��������
		key�ظ�ʱ���ر�������ELEMENT->data,���û��л����ͷ��ڴ�
		data������ʹ�ú�Tree����ͬ�ķ�������new������ָ��
	*/
	Tree::ELEMENT* insert( Tree::ELEMENT *data );
	//ɾ�����ݷ��ر�ɾ��������
	void* erase( void *key, short keySize );
	void* erase( void *key, short keySize, unsigned int hashKey );//�����ܽӿ�

	//����Ԫ�رȽϷ���
	void setDataCmp(int dataCmp( void *left, void *right));
	//����������
	//���öѷ�������������ڴ��������
	//n��Ԫ����Ҫn��Tree::TREE��n��Tree::ELEMENT
	void setHeapFun( void* (*allocFun)(int size), void (*freeFun)(void* pVoid) );

private:
	unsigned int hash( void *key, short keySize );//hash�㷨
	/*
		�Ƚ�
		Left>Right return > 0
		Left=Right return 0
		Left<Right return < 0
	*/
	int keycmp( char *left, short leftSize, unsigned int leftHash, char *right, short rightSize, unsigned int rightHash );
	void release(Tree::TREE *pTree);//�ͷ���ռ�õ��ڴ�

	static void* AllocMethod(int size);
	static void FreeMethod(void* pVoid);

private:
	TREE	*m_tree;
	int		m_size;

	void* (*alloc)(int size);
	void (*free)(void* pVoid);
};

}

#endif //TREE_H
