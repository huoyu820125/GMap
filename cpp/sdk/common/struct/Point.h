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
//����
class Point : public CanLocateData
{
public:
	static Grid::Point* Alloc();
	static void Free(Grid::Point *pPoint);

	Point();
	virtual ~Point();
	bool AddLine(bigint lineId, bool isOut);	//��ӳ���
	bool DelLine(bigint lineId, bool isOut);//ɾ����
	Grid::Result::Result UpdateFields(Grid::Point *pPoint);//�޸����ԣ��������޸ģ������������
	void Release();//�ͷ��ڴ�

	bigint		id;//����id
	mdk::Map	data;//����string, FIELD*
	OwnLines	out;//����
	OwnLines	in;//���
	//////////////////////////////////////////////////////////////////////////
	//�־û�ҵ��������
	int			diskSpace;//ռ�õ�Ӳ�̿ռ�

	//////////////////////////////////////////////////////////////////////////
	//����ҵ��������
	bigint		searchNo;//���1�α��������ļ��������ı��
	time_t		searchTime;//����������ʱ��
};

}

#endif //POINT_H
