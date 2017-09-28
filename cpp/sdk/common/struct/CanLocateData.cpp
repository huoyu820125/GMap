#include "CanLocateData.h"
#include <cstring>

namespace Grid
{
	CanLocateData::CanLocateData()
	{
		char *buf = (char*)this;
		buf -= INDEX_SIZE;//移动到地址起始位置
		buf[0] = 0xff;//索引位置设置为<0
	}

	CanLocateData::~CanLocateData(){}

	bool CanLocateData::IsPositioned()
	{
		if ( 0 > IdxPos() ) return false;

		return true;
	}

	void CanLocateData::SetIdxPos(bigint pos)
	{
		char *buf = (char*)this;
		buf -= INDEX_SIZE;//移动到地址起始位置
		memcpy(buf, &pos, sizeof(bigint));//保存索引位置
	}

	bigint CanLocateData::IdxPos()
	{
		char *buf = (char*)this;
		buf -= INDEX_SIZE;//移动到地址起始位置
		bigint pos;
		memcpy(&pos, buf, sizeof(bigint));//取索引位置
		return pos;
	}

	void CanLocateData::SetDataPos(bigint pos)
	{
		char *buf = (char*)this;
		buf -= INDEX_SIZE;//移动到地址起始位置
		memcpy(&buf[sizeof(bigint)], &pos, sizeof(bigint));//保存数据位置
	}

	bigint CanLocateData::DataPos()
	{
		char *buf = (char*)this;
		buf -= INDEX_SIZE;//移动到地址起始位置
		bigint pos;
		memcpy(&pos, &buf[sizeof(bigint)], sizeof(bigint));//取数据位置
		return pos;
	}

}

