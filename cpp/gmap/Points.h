#ifndef POINTS_H
#define POINTS_H

#include "common/struct/Point.h"
#include <map>
#include "mdk_ex/RHTable.h"

class GridStore;
class DiskIO;
class Points
{
public:
	Points(GridStore *pStore);
	virtual ~Points();
	const char* LoadData(int &count);//加载数据
	Grid::Result::Result SetPoint(Grid::Point &point);
	Grid::Point* FindPoint(bigint id);
	Grid::Point* AddLine(bigint pointId, bigint lineId, bool isOut);//添加边,返回被修改的顶点，顶点不存在返回NULL
	Grid::Point* DelLine(bigint pointId, bigint lineId, bool isOut);//删除边,返回被修改的顶点
	Grid::Result::Result DelPoint(bigint pointId);//删除顶点
	bool SavePoint(Grid::Point *pPoint);//保存顶点数据
	//////////////////////////////////////////////////////////////////////////
	//批量操作
	//创建1批顶点
	Grid::Result::Result CreatePoints(std::vector<Grid::Point> &points);
	
private:
	Grid::Result::Result UpdatePoint(Grid::Point *dest, Grid::Point *src);

private:
	GridStore						*m_pStore;//数据库
	std::map<bigint, Grid::Point*>	m_points;//顶点集合
	DiskIO							*m_pDisk;//硬盘
};

#endif //POINTS_H