#ifndef POINT_H
#define POINT_H

#include <vector>
#include <map>
#include "CanLocateData.h"
#include "common/Grid.h"
#include "mdk_ex/container/Map.h"
#include "OwnLines.h"


namespace Grid
{
//顶点
class Point : public CanLocateData
{
public:
	static Grid::Point* Alloc();
	static void Free(Grid::Point *pPoint);

	Point();
	virtual ~Point();
	bool AddLine(bigint lineId, bool isOut);	//添加出边
	bool DelLine(bigint lineId, bool isOut);//删除边
	Grid::Result::Result UpdateFields(Grid::Point *pPoint);//修改属性，存在则修改，不存在则添加
	void Release();//释放内存

	bigint		id;//顶点id
	mdk::Map	data;//数据string, FIELD*
	OwnLines	out;//出边
	OwnLines	in;//入边
	//////////////////////////////////////////////////////////////////////////
	//持久化业务辅助属性
	int			diskSpace;//占用的硬盘空间

	//////////////////////////////////////////////////////////////////////////
	//搜索业务辅助属性
	bigint		searchNo;//最后1次被检索到的检索操作的编号
	time_t		searchTime;//被搜索到的时间
};

}

#endif //POINT_H
