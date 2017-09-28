#ifndef LINE_H
#define LINE_H

#include "CanLocateData.h"
#include "common/Grid.h"
#include "mdk_ex/container/Map.h"


namespace Grid
{
	class Point;
	//��
	class Line : public CanLocateData
	{
	public:
		static Grid::Line* Alloc();
		static void Free(Grid::Line *pLine);

		Line();
		~Line();
		Grid::Result::Result UpdateFields(Grid::Line *pLine);
		void Release();//�ͷ��ڴ�
		bigint	id;
		bigint	startId;//���id
		bigint	endId;//�յ�id

		mdk::Map	data;//����string, FIELD*
		//////////////////////////////////////////////////////////////////////////
		//�־û�ҵ��������
		int			diskSpace;//ռ�õ�Ӳ�̿ռ�

		//////////////////////////////////////////////////////////////////////////
		//����ҵ��������
		Point	*pStartPoint;//��㣬����������ݹ�����ͬ����ֱ��ָ�򱾵��ڴ棬����ΪNULL
		Point	*pEndPoint;//�յ㣬����������ݹ�����ͬ����ֱ��ָ�򱾵��ڴ棬����ΪNULL
	};
}

#endif //LINE_H
