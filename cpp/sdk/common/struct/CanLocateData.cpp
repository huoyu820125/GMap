#include "CanLocateData.h"
#include <cstring>

namespace Grid
{
	CanLocateData::CanLocateData()
	{
		char *buf = (char*)this;
		buf -= INDEX_SIZE;//�ƶ�����ַ��ʼλ��
		buf[0] = 0xff;//����λ������Ϊ<0
	}

	CanLocateData::~CanLocateData(){}

	bool CanLocateData::IsPositioned()
	{
		if ( 0 > IdxPos() ) return false;

		return true;
	}

	void CanLocateData::SetIdxPos(bigint pos)
	{
		char *buf = (char*)this;
		buf -= INDEX_SIZE;//�ƶ�����ַ��ʼλ��
		memcpy(buf, &pos, sizeof(bigint));//��������λ��
	}

	bigint CanLocateData::IdxPos()
	{
		char *buf = (char*)this;
		buf -= INDEX_SIZE;//�ƶ�����ַ��ʼλ��
		bigint pos;
		memcpy(&pos, buf, sizeof(bigint));//ȡ����λ��
		return pos;
	}

	void CanLocateData::SetDataPos(bigint pos)
	{
		char *buf = (char*)this;
		buf -= INDEX_SIZE;//�ƶ�����ַ��ʼλ��
		memcpy(&buf[sizeof(bigint)], &pos, sizeof(bigint));//��������λ��
	}

	bigint CanLocateData::DataPos()
	{
		char *buf = (char*)this;
		buf -= INDEX_SIZE;//�ƶ�����ַ��ʼλ��
		bigint pos;
		memcpy(&pos, &buf[sizeof(bigint)], sizeof(bigint));//ȡ����λ��
		return pos;
	}

}

