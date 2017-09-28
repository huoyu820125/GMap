#ifndef CANLOCATEDATA_H
#define CANLOCATEDATA_H

#include <vector>
#include <map>
#include "common/Grid.h"


namespace Grid
{
	/*
		可定位的数据
		只适用于使用AllocPoint，AllocLine创建的对象
	*/
	class CanLocateData
	{
	public:
		CanLocateData();
		virtual ~CanLocateData();
		bool IsPositioned();//已定位数据在文件中的位置，否则表示数据是新建数据，尚未确定在文件中的位置
		void SetIdxPos(bigint pos);//记录索引数据在索引文件中的位置
		void SetDataPos(bigint pos);//记录数据在数据文件中的文职
		bigint IdxPos();//索引数据在索引文件中的位置
		bigint DataPos();//数据在数据文件中的文职
	};
}

#endif //CANLOCATEDATA_H
