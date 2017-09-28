#ifndef LINES_H
#define LINES_H

#include "common/struct/Line.h"
#include <map>

class GridStore;
class DiskIO;

class Lines
{
public:
	Lines(GridStore *pStore);
	virtual ~Lines();
	const char* LoadData(int &count);//加载数据
	//使用直线line链接2个顶点startPoint和endPoint
	Grid::Result::Result CreateLine(Grid::Line &line, Grid::Point *pStartPoint, Grid::Point *pEndPoint);
	Grid::Line* FindLine(bigint lineId);
	Grid::Result::Result DelLine(bigint lineId, bigint &startId, bigint &endId);

private:
	void UpdateLine(Grid::Line *dest, Grid::Line *src);

private:
	DiskIO	*m_pDisk;//硬盘
	GridStore						*m_pStore;//数据库
	std::map<bigint, Grid::Line*>	m_lines;//边集合
};

#endif //LINES_H