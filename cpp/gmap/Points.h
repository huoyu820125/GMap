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
	const char* LoadData(int &count);//��������
	Grid::Result::Result SetPoint(Grid::Point &point);
	Grid::Point* FindPoint(bigint id);
	Grid::Point* AddLine(bigint pointId, bigint lineId, bool isOut);//��ӱ�,���ر��޸ĵĶ��㣬���㲻���ڷ���NULL
	Grid::Point* DelLine(bigint pointId, bigint lineId, bool isOut);//ɾ����,���ر��޸ĵĶ���
	Grid::Result::Result DelPoint(bigint pointId);//ɾ������
	bool SavePoint(Grid::Point *pPoint);//���涥������
	//////////////////////////////////////////////////////////////////////////
	//��������
	//����1������
	Grid::Result::Result CreatePoints(std::vector<Grid::Point> &points);
	
private:
	Grid::Result::Result UpdatePoint(Grid::Point *dest, Grid::Point *src);

private:
	GridStore						*m_pStore;//���ݿ�
	std::map<bigint, Grid::Point*>	m_points;//���㼯��
	DiskIO							*m_pDisk;//Ӳ��
};

#endif //POINTS_H