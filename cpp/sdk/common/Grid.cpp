#include "Grid.h"

namespace Grid
{

namespace Result
{

char* Reason(Result result)
{
	if ( paramError == result ) return "��������";
	else if ( notOwner == result ) return "�ڵ㲻������ӵ����";
	else if ( noData == result ) return "������";
	else if ( startError == result ) return "������";
	else if ( endError == result ) return "�յ����";
	else if ( fileError == result ) return "�ļ�����";
	else if ( noIdSource == result ) return "�޿���id";
	else if ( tooManyField == result ) return "�ֶ�̫��";
	else if ( tooBigFieldName == result ) return "�ֶ���̫��";
	else if ( tooBigField == result ) return "�ֶγ���̫��";

	else if ( noDataDir == result ) return "û������Ŀ¼";
	else if ( noPermissions == result ) return "���ļ����ļ���Ȩ��";
	else if ( noFile == result ) return "�ļ�������";
	else if ( noDiskSpace == result ) return "���̲���";
	else if ( dataDamage == result ) return "������";
	else if ( noMemery == result ) return "�ڴ治��";
	else if ( idxDamage == result ) return "����������";
	else if ( idxError == result ) return "��������";

	return "δ֪����";
}

}

//Ѱ�����ݱ������ĸ��ڵ���
int FindNodeId(bigint dataId, int nodeCount)
{
	return (dataId % nodeCount) + 1;
}

//�ڵ�(nodeId)�����ݵ�ӵ���ߣ����ݴ洢�ڽڵ�(nodeId)�ϣ�
bool IsOwner(bigint dataId, int nodeId, int nodeCount)
{
	return nodeId == FindNodeId(dataId, nodeCount);
}

}
