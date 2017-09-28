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
	const char* LoadData(int &count);//��������
	//ʹ��ֱ��line����2������startPoint��endPoint
	Grid::Result::Result CreateLine(Grid::Line &line, Grid::Point *pStartPoint, Grid::Point *pEndPoint);
	Grid::Line* FindLine(bigint lineId);
	Grid::Result::Result DelLine(bigint lineId, bigint &startId, bigint &endId);

private:
	void UpdateLine(Grid::Line *dest, Grid::Line *src);

private:
	DiskIO	*m_pDisk;//Ӳ��
	GridStore						*m_pStore;//���ݿ�
	std::map<bigint, Grid::Line*>	m_lines;//�߼���
};

#endif //LINES_H