#ifndef GRID_H
#define GRID_H

#include <map>
#include <string>
#include <vector>

#ifdef WIN32
typedef __int64				bigint;
#else
#include <sys/types.h>
typedef int64_t				bigint;
#endif

#define INDEX_SIZE (sizeof(bigint) + sizeof(bigint) + sizeof(int))//�������ȣ�����id+�����ļ���λ��+���ݳ���

namespace Grid
{
	//�洢����
	namespace Limit
	{
		enum Limit
		{
			/*
				�������ռ�ã�
					835byte��
						����״̬(char) + ����id(int64) + ��������(int) + �������(int) + �ֶ�����(short)
						+ max�ֶ���(10) * 
						(�ֶ�������(short) + �ֶ���(16 + 1) + �ֶ�����(char) + �ֶ�ֵ����(int) + �ֶ�ֵ(64))
					max18963byte��
						����״̬(char) + ����id(int64) + ��������(int) + �������(int) + �ֶ�����(short)
						+ max�ֶ���(64) * 
						(�ֶ�������(short) + �ֶ���(32 + 1) + �ֶ�����(char) + �ֶ�ֵ����(int) + �ֶ�ֵ(256))

				��maxռ�ô���18971byte��
					195byte��
						����״̬(char) + ����id(int64) + ���id(int64) + �յ�id(int64) + �ֶ�����(short)
						+ max�ֶ���(3) * 
						(�ֶ�������(short) + �ֶ���(16 + 1) + �ֶ�����(char) + �ֶ�ֵ����(int) + �ֶ�ֵ(32))
					max18971byte��
						����״̬(char) + ����id(int64) + ���id(int64) + �յ�id(int64) + �ֶ�����(short)
						+ max�ֶ���(64) * 
						(�ֶ�������(short) + �ֶ���(32 + 1) + �ֶ�����(char) + �ֶ�ֵ����(int) + �ֶ�ֵ(256))

				10�ڶ���+1000�����ߣ�maxռ�ô���byte��
					min:
						835*10��+195*1000��+8*1000��+8*1000��
						16T=16224G=778G+15446G
						���������ļ�2.6G= 16224G / 3�ڵ� / 8�� / 256�ļ���
						0.88020833333333333333333333333333
					max:
						18963*10��+18971*1000��+8*1000��+8*1000��
						1785084G=1767424G+17660G
			*/
			maxFieldCount = 64,//����ͱ�������е�����ֶ���
			maxFieldNameSize = 32,//�ֶ�����󳤶�
			maxFieldSize = 256,//�ֶ���󳤶�
			maxDBShardCount = 256,//�����ļ�����Ƭ��
			maxFilter = 16,//��������������
			maxMoveCount = 10,//�ƶ���󾭹�����
			maxObjectCount = 1000000,//����������������100��
		};
	}

	namespace Result
	{
		enum Result
		{
			success = 0,
			paramError = 1,	//��������
			notOwner = 2,//�ڵ㲻������ӵ����
			noData = 3,//������
			startError = 4,	//������
			endError = 5,	//�յ����
			fileError = 6,//�ļ�����
			noIdSource = 7,//�޿���id
			tooManyField = 8,//�ֶ�̫��
			tooBigFieldName = 9,//�ֶ���̫��
			tooBigField = 10,//�ֶγ���̫��

			//DiskIO����
			noDataDir = 11,//û������Ŀ¼
			noPermissions = 12,//���ļ����ļ���Ȩ��
			noFile = 13,//�ļ�������
			noDiskSpace = 14,//���̲���
			dataDamage = 15,//������
			noMemery = 16,//�ڴ治��
			idxDamage = 17,//����������
			idxError = 18,//��������

		};
		char* Reason(Result result);
	}

	//���Ͷ���
	enum DataType
	{
		undef = 0,//δ��������
		int8 = 1,//8�ֽ����� char byte bool
		int16 = 2,//2�ֽ����� short
		int32 = 3,//4�ֽ����� int
		int64 = 4,//8�ֽ����� bigint
		str = 5,//�ַ���
		date = 6,//����
	};

	//�ֶ�
	typedef struct FIELD
	{
		DataType		type;
		bigint			value;//������������
		char			data[Limit::maxFieldSize+1];//�����ַ��������л���������
		int				size;//���ݳ���
	}FIELD;

	//move�������Զ���Ĳ���
	enum Action
	{
		get = 0,//ȡ�ö�������(���move����յ�)
		count = 1,//ͳ�ƶ�������(���move����յ�)
		del = 2,//ɾ������(1��move�յ�)
		unlink = 3,//�Ͽ�����(1��move)
	};

	//�ȽϷ�ʽ
	enum CmpMode
	{
		left = -2, //<
		leftEquals = -1, //<=
		equals = 0,//=
		rightEquals = 1,//>
		right = 2,//>=
		unequals = 3,//!=
	};
	//ѡ��
	typedef struct FILTER
	{
		char			cmpMode;//�ȽϷ�ʽ��enum CmpMode
		std::string		fieldName;//�ֶ���
		unsigned int	hashValue;//hashֵ
		FIELD			field;//�ֶ�
	}FILTER;
	//move����1���ƶ�ѡ��ıߣ�ƥ������
	typedef struct SELECT_LINE
	{
		bool moveOut;//ѡ����move
		bool moveAllLine;//��moveOutָ���ķ���������·���ƶ���moveAllLine = trueʱ��selectField��Ч
		std::vector<Grid::FILTER> selectField;//ѡƥ���ֶεı�
	}SELECT_LINE;

	//�ڵ�(nodeId)�����ݵ�ӵ���ߣ����ݴ洢�ڽڵ�(nodeId)�ϣ�
	bool IsOwner(bigint dataId, int nodeId, int nodeCount);
	//Ѱ�����ݱ������ĸ��ڵ���
	int FindNodeId(bigint dataId, int nodeCount);

	//Զ�̶���
	typedef struct REMOTE_DATA{
		int lineIndex;
		std::vector<bigint> lineIds;
		std::vector<bigint> pointIds;
	}REMOTE_DATA;

}

#endif //GRID_H
