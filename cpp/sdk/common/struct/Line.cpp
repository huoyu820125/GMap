#include "Line.h"
#include <cstring>
#include "common/heap.h"

namespace Grid
{

Grid::Line* Line::Alloc()
{
	char* buffer = (char*)heap::Alloc(INDEX_SIZE + sizeof(Grid::Line));
	return new (&buffer[INDEX_SIZE])Grid::Line;
}

void Line::Free(Grid::Line *pLine)
{
	pLine->Release();
	char *buffer = (char*)pLine;
	buffer -= INDEX_SIZE;
	heap::Free(buffer);
	return;
}

Line::Line()
{
	pStartPoint = NULL;
	pEndPoint = NULL;
}

Line::~Line()
{
}

void Line::Release()
{
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

Grid::Result::Result Line::UpdateFields(Grid::Line *pLine)
{
	if (pLine == this) return Grid::Result::success;

	//检查属性数量
	int fieldCount = data.size();
	mdk::Map::iterator it;
	mdk::Map::iterator itNew;
	for ( itNew = pLine->data.begin(); !itNew.isEnd(); itNew++ )
	{
		if ( itNew.key()->size > Grid::Limit::maxFieldNameSize ) return Grid::Result::tooBigFieldName;
		it = data.find(itNew.key()->data, itNew.key()->size, itNew.key()->hashKey);
		if ( it.isEnd() ) fieldCount++;//需要增加字段
		if ( fieldCount > Grid::Limit::maxFieldCount ) return Grid::Result::tooManyField;
	}

	//复制属性
	Grid::FIELD *pField = NULL;
	Grid::FIELD *pNewField = NULL;
	for ( itNew = pLine->data.begin(); !itNew.isEnd(); itNew++ )
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

	//不检查被pLine中缺少的字段，缺少的字段认为保留原始值

	return Grid::Result::success;
}

}

