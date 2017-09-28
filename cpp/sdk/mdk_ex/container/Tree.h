#ifndef TREE_H
#define TREE_H

namespace mdk
{

//二叉搜索树
//采用伪删除
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
		int		index;//插入顺序号，用于重复数据判断新旧
	};

	typedef struct TREE
	{
		ELEMENT	*e;
		bool	isDeleted;
		TREE	*left;
		TREE	*right;
		bool	ownerElement;
	}TREE;

	void release();//释放内存
	void clear();//清空数据，不释放内存
	int size();
	void* find( void *key, short keySize );
	void* find( char *key, short keySize, unsigned int hashKey );//高性能接口
	Tree::TREE* findNode( char *key, short keySize, unsigned int hashKey );//高性能接口
	/*
		data必须是new
		key重复时，返回被放弃的数据
	*/
	void* insert( void *key, short keySize, void *data );//重复返回已存在的元素
	void* insert( char *key, short keySize, unsigned int hashKey, void *data );//高性能接口
	/*
		直接插入创建好的元素，用于多个容器共享同一类型数据的场景
		减少copy，降低内存占用，提高性能
		key重复时返回被放弃的ELEMENT->data,让用户有机会释放内存
		data必须是使用和Tree类相同的分配器上new出来的指针
	*/
	Tree::ELEMENT* insert( Tree::ELEMENT *data );
	//删除数据返回被删除的数据
	void* erase( void *key, short keySize );
	void* erase( void *key, short keySize, unsigned int hashKey );//高性能接口

	//设置元素比较方法
	void setDataCmp(int dataCmp( void *left, void *right));
	//高性能设置
	//设置堆方法，用于提高内存分配性能
	//n个元素需要n个Tree::TREE和n个Tree::ELEMENT
	void setHeapFun( void* (*allocFun)(int size), void (*freeFun)(void* pVoid) );

private:
	unsigned int hash( void *key, short keySize );//hash算法
	/*
		比较
		Left>Right return > 0
		Left=Right return 0
		Left<Right return < 0
	*/
	int keycmp( char *left, short leftSize, unsigned int leftHash, char *right, short rightSize, unsigned int rightHash );
	void release(Tree::TREE *pTree);//释放树占用的内存

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
