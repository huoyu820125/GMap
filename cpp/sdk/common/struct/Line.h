#ifndef LINE_H
#define LINE_H

#include "CanLocateData.h"
#include "common/Grid.h"
#include "mdk_ex/container/Map.h"


namespace Grid
{
	class Point;
	//边
	class Line : public CanLocateData
	{
	public:
		static Grid::Line* Alloc();
		static void Free(Grid::Line *pLine);

		Line();
		~Line();
		Grid::Result::Result UpdateFields(Grid::Line *pLine);
		void Release();//释放内存
		bigint	id;
		bigint	startId;//起点id
		bigint	endId;//终点id

		mdk::Map	data;//数据string, FIELD*
		//////////////////////////////////////////////////////////////////////////
		//持久化业务辅助属性
		int			diskSpace;//占用的硬盘空间

		//////////////////////////////////////////////////////////////////////////
		//搜索业务辅助属性
		Point	*pStartPoint;//起点，如果顶点数据归属相同，则直接指向本地内存，否则为NULL
		Point	*pEndPoint;//终点，如果顶点数据归属相同，则直接指向本地内存，否则为NULL
	};
}

#endif //LINE_H
