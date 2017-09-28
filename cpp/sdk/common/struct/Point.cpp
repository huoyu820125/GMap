#include "Point.h"
#include <cstring>
#include "common/heap.h"

namespace Grid
{

Grid::Point* Point::Alloc()
{
	char* buffer = (char*)heap::Alloc(INDEX_SIZE + sizeof(Grid::Point));
	return new (&buffer[INDEX_SIZE])Grid::Point;
}

void Point::Free(Grid::Point *pPoint)
{
	pPoint->Release();
	char *buffer = (char*)pPoint;
	buffer -= INDEX_SIZE;
	heap::Free(buffer);
	return;
}

Point::Point()
{
	//不初始化数据，可能从内存池创建对象，没机会调用构造函数
}

Point::~Point()
{
}

void Point::Release()
{
	out.Release();
	in.Release();
	mdk::Map::iterator it = data.begin();
	for ( ; !it.isEnd(); it++ ) 
	{
		if ( NULL != it.data() )
		{
			delete it.data();
		}
	}
	data.release();
}

bool Point::AddLine(bigint lineId, bool isOut)
{
	if ( 0 >= lineId ) return false;
	//不检查是否重复
	if ( isOut ) out.Add(lineId);
	else in.Add(lineId);
	return true;
}

bool Point::DelLine(bigint lineId, bool isOut)
{
	if ( isOut ) return out.Del(lineId);
	else return in.Del(lineId);
}

Grid::Result::Result Point::UpdateFields(Grid::Point *pPoint)
{
	if (pPoint == this) return Grid::Result::success;

	//顶点是独立个体id不能修改，只复制数据
	//不复顶点之间的连接（边 Grid::Line）

	//检查属性数量
	int fieldCount = data.size();
	mdk::Map::iterator it;
	mdk::Map::iterator itNew;
	for ( itNew = pPoint->data.begin(); !itNew.isEnd(); itNew++ )
	{
		if ( itNew.key()->size > Grid::Limit::maxFieldNameSize ) return Grid::Result::tooBigFieldName;
		it = data.find(itNew.key()->data, itNew.key()->size, itNew.key()->hashKey);
		if ( it.isEnd() ) fieldCount++;//需要增加字段
		if ( fieldCount > Grid::Limit::maxFieldCount ) return Grid::Result::tooManyField;
	}

	//复制属性
	Grid::FIELD *pField = NULL;
	Grid::FIELD *pNewField = NULL;
	for ( itNew = pPoint->data.begin(); !itNew.isEnd(); itNew++ )
	{
		pNewField = (Grid::FIELD*)itNew.data();
		if ( pNewField->size > Grid::Limit::maxFieldSize ) return Grid::Result::tooBigField;
		it = data.find(itNew.key()->data, itNew.key()->size, itNew.key()->hashKey);
		if ( it.isEnd() )//创建字段
		{
			pField = new Grid::FIELD;
			data.insert(itNew.key()->data, itNew.key()->size, itNew.key()->hashKey, pField);
		}
		else pField = (Grid::FIELD*)it.data();

		pField->type = pNewField->type;//允许更改字段类型
		if ( Grid::str != pField->type )
		{
			pField->value = pNewField->value;
			continue;
		}

		pField->size = pNewField->size;
		memcpy(pField->data, pNewField->data, pField->size);
	}

	//不检查被pPoint中缺少的字段，缺少的字段认为保留原始值

	return Grid::Result::success;
}

}

