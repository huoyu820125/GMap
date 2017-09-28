#ifndef CANLOCATEDATA_H
#define CANLOCATEDATA_H

#include <vector>
#include <map>
#include "common/Grid.h"


namespace Grid
{
	/*
		�ɶ�λ������
		ֻ������ʹ��AllocPoint��AllocLine�����Ķ���
	*/
	class CanLocateData
	{
	public:
		CanLocateData();
		virtual ~CanLocateData();
		bool IsPositioned();//�Ѷ�λ�������ļ��е�λ�ã������ʾ�������½����ݣ���δȷ�����ļ��е�λ��
		void SetIdxPos(bigint pos);//��¼���������������ļ��е�λ��
		void SetDataPos(bigint pos);//��¼�����������ļ��е���ְ
		bigint IdxPos();//���������������ļ��е�λ��
		bigint DataPos();//�����������ļ��е���ְ
	};
}

#endif //CANLOCATEDATA_H
