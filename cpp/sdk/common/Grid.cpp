#include "Grid.h"

namespace Grid
{

namespace Result
{

char* Reason(Result result)
{
	if ( paramError == result ) return "参数错误";
	else if ( notOwner == result ) return "节点不是数据拥有者";
	else if ( noData == result ) return "无数据";
	else if ( startError == result ) return "起点错误";
	else if ( endError == result ) return "终点错误";
	else if ( fileError == result ) return "文件错误";
	else if ( noIdSource == result ) return "无可用id";
	else if ( tooManyField == result ) return "字段太多";
	else if ( tooBigFieldName == result ) return "字段名太长";
	else if ( tooBigField == result ) return "字段长度太大";

	else if ( noDataDir == result ) return "没有数据目录";
	else if ( noPermissions == result ) return "无文件或文件夹权限";
	else if ( noFile == result ) return "文件不存在";
	else if ( noDiskSpace == result ) return "磁盘不足";
	else if ( dataDamage == result ) return "数据损坏";
	else if ( noMemery == result ) return "内存不足";
	else if ( idxDamage == result ) return "索引数据损坏";
	else if ( idxError == result ) return "索引错误";

	return "未知错误";
}

}

//寻找数据保存在哪个节点上
int FindNodeId(bigint dataId, int nodeCount)
{
	return (dataId % nodeCount) + 1;
}

//节点(nodeId)是数据的拥有者（数据存储在节点(nodeId)上）
bool IsOwner(bigint dataId, int nodeId, int nodeCount)
{
	return nodeId == FindNodeId(dataId, nodeCount);
}

}
