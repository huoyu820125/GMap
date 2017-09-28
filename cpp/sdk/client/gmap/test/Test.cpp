// ClusterCgCli.cpp : 定义控制台应用程序的入口点。
//

#ifdef WIN32
#ifdef _DEBUG
#pragma comment ( lib, "mdk_d.lib" )
//#pragma comment ( lib, "hiredis_d.lib" )
#else
#pragma comment ( lib, "mdk.lib" )
// #pragma comment ( lib, "hiredis.lib" )
#endif
#endif

#include "../GCluster.h"
#include "mdk/include/mdk/mapi.h"
#define Node1_IP "192.168.2.225"
#define Node1_Port 6801
#define Node2_IP "127.0.0.1"
#define Node2_Port 6802
#define Node3_IP "127.0.0.1"
#define Node3_Port 6803
#define Node4_IP "127.0.0.1"
#define Node4_Port 6804
#define Node5_IP "127.0.0.1"
#define Node5_Port 6805
#define Node6_IP "127.0.0.1"
#define Node6_Port 6806

enum LinkType
{
	UNKNOW = 0,
	Friend = 1,
	Colleague = 2,
	Kinsfolk = 3,
	Schoolmate = 4,
	Blacklist = 5,
	Whitelist = 6,
};

class TypeDes
{
public:
	TypeDes()
	{
		m_des[Friend] = "好友";
		m_des[Colleague] = "同事";
		m_des[Kinsfolk] = "亲戚";
		m_des[Schoolmate] = "同学";
		m_des[Blacklist] = "黑名单";
		m_des[Whitelist] = "白名单";
	}
	~TypeDes()
	{

	}

	std::map<LinkType, std::string> m_des;
};

TypeDes g_des;

void LinkPoints(bigint startPointId, std::vector<bigint> &endPoints, int startPos, int linkCount, LinkType linkType);
void TestMap(std::vector<bigint> &points);

void TestLine(std::vector<bigint> &points);
void TestPoint()
{
	GCluster grid;
	NODE node;
	node.nodeId = 1;
	node.ip = Node1_IP;
	node.port = Node1_Port;
	grid.AddNode(node);
// 	node.nodeId = 2;
// 	node.ip = Node2_IP;
// 	node.port = Node2_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 3;
// 	node.ip = Node3_IP;
// 	node.port = Node3_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 4;
// 	node.ip = Node4_IP;
// 	node.port = Node4_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 5;
// 	node.ip = Node5_IP;
// 	node.port = Node5_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 6;
// 	node.ip = Node6_IP;
// 	node.port = Node6_Port;
// 	grid.AddNode(node);

	Grid::Point point;
	Grid::FIELD *pCoin = new Grid::FIELD;
	point.data.insert("coin", strlen("coin"), pCoin);
	pCoin->type = Grid::int32;
	Grid::FIELD *pName = new Grid::FIELD;
	point.data.insert("name", strlen("name"), pName);
	pName->type = Grid::str;
	Grid::FIELD *pId = new Grid::FIELD;
	point.data.insert("id", strlen("id"), pId);
	pId->type = Grid::int32;


	int i = 0;
	CallResult ret;
	std::vector<bigint> points;
	mdk::uint64 start = mdk::MillTime();
	Grid::Point replyPoint;
	for ( i = 0; i < 10000; i++ )
	{
		point.id = 0;
		sprintf(pName->data, "point%d", i);
// 		pName->size = strlen(pName->data) + 1;
		pId->value = rand();
		pCoin->value = rand();
		ret = grid.SetPoint(point);
		if ( !ret.isSuccess )
		{
			printf("%s\n", ret.reason.c_str());
		}
		std::vector<Grid::Point> rp; 
		std::vector<bigint> ids;
		std::vector<Grid::SELECT_LINE> selectLines;
		std::vector<Grid::FILTER> pointFilter;
		bool selectAll;
		std::vector<std::string> getFields;
		Grid::Action act;
		ids.push_back(point.id);
		mdk::uint64 startMove = mdk::MillTime();
		ret = grid.Move(rp, ids, selectLines, pointFilter, true, getFields, Grid::get);
		if ( !ret.isSuccess )
		{
			printf("%s\n", ret.reason.c_str());
		}
		startMove = mdk::MillTime() - startMove;
		startMove = mdk::MillTime();
		ret = grid.GetPoint(point.id, replyPoint);
		if ( !ret.isSuccess )
		{
			printf("%s\n", ret.reason.c_str());
		}
		startMove = mdk::MillTime() - startMove;
		replyPoint.Release();
		points.push_back(point.id);
		pId->value = i;
		pCoin->value = i*10;
		sprintf(pName->data, "point%du", i);
// 		pName->size = strlen(pName->data) + 1;
		ret = grid.SetPoint(point);
		if ( !ret.isSuccess )
		{
			printf("%s\n", ret.reason.c_str());
		}
		ret = grid.GetPoint(point.id, replyPoint);
		if ( !ret.isSuccess )
		{
			printf("%s\n", ret.reason.c_str());
		}
		replyPoint.Release();
// 		ret = grid.DelPoint(point.id);
// 		if ( !ret.isSuccess )
// 		{
// 			printf("%s\n", ret.reason.c_str());
// 		}
// 		ret = grid.GetPoint(point.id, replyPoint);
// 		if ( ret.isSuccess )
// 		{
// 			printf("%s\n", ret.reason.c_str());
// 			replyPoint.Release();
// 		}
	}
	start = mdk::MillTime() - start;
	int count = 40000.0 * 1000.0 / start;
	printf("tps = %d\n", count);
	/*
		0与1~2000连接类型=1
		0与2001~4000连接类型=2
		0与4001~6000连接类型=3

		1与1001~3000连接类型=1
		1与3001~5000连接类型=2

		查询1，2建立1类型连接的顶点
		结果
		1~2000
		1001~2000 重复
		2001~3000
	*/
	LinkPoints(points[0], points, 1, 2000, LinkType::Friend);
	LinkPoints(points[0], points, 2001, 2000, LinkType::Schoolmate);
	LinkPoints(points[0], points, 4001, 2000, LinkType::Blacklist);
	LinkPoints(points[1], points, 1001, 2000, LinkType::Friend);
	LinkPoints(points[1], points, 3001, 2000, LinkType::Schoolmate);
	//2级连接
// 	for ( i = 1; i < 1001; i++ )
// 	{
// 		LinkPoints(points[i], points, 5000 + i, 1, LinkType::Schoolmate);
// 	}
// 	for ( i = 2001; i < 3001; i++ )
// 	{
// 		LinkPoints(points[i], points, 3500 + i, 1, LinkType::Schoolmate);
// 	}

	TestMap(points);
// 	TestMap(points);
// 	TestLine(points);
}

void LinkPoints(bigint startPointId, std::vector<bigint> &endPoints, int startPos, int linkCount, LinkType linkType)
{
	GCluster grid;
	NODE node;
	node.nodeId = 1;
	node.ip = Node1_IP;
	node.port = Node1_Port;
	grid.AddNode(node);
// 	node.nodeId = 2;
// 	node.ip = Node2_IP;
// 	node.port = Node2_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 3;
// 	node.ip = Node3_IP;
// 	node.port = Node3_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 4;
// 	node.ip = Node4_IP;
// 	node.port = Node4_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 5;
// 	node.ip = Node5_IP;
// 	node.port = Node5_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 6;
// 	node.ip = Node6_IP;
// 	node.port = Node6_Port;
// 	grid.AddNode(node);

	Grid::Line line;
	Grid::FIELD *pType = new Grid::FIELD;
	line.data.insert("关系id", strlen("关系id"), pType);
	pType->type = Grid::int32;
	pType->value = linkType;
	Grid::FIELD *pTypeDes = new Grid::FIELD;
	line.data.insert("关系", strlen("关系"), pTypeDes);
	pTypeDes->type = Grid::str;
	sprintf(pTypeDes->data, "%s", g_des.m_des[linkType].c_str());

	CallResult ret;
	Grid::Line replyLine;
	//将顶点与startPos位置开始的linkCount个顶点相连
	int i = 0;
	for ( i = startPos; i < startPos + linkCount; i++ )
	{
		line.id = 0;
		line.startId = startPointId;
		line.endId = endPoints[i];
		ret = grid.Link(line);
		if ( !ret.isSuccess )
		{
			printf("%s\n", ret.reason.c_str());
		}
		ret = grid.GetLine(line.id, replyLine);
		if ( !ret.isSuccess )
		{
			printf("%s\n", ret.reason.c_str());
		}
		replyLine.Release();
	}
}

void TestMap(std::vector<bigint> &points)
{
	GCluster grid;
	NODE node;
	node.nodeId = 1;
	node.ip = Node1_IP;
	node.port = Node1_Port;
	grid.AddNode(node);
// 	node.nodeId = 2;
// 	node.ip = Node2_IP;
// 	node.port = Node2_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 3;
// 	node.ip = Node3_IP;
// 	node.port = Node3_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 4;
// 	node.ip = Node4_IP;
// 	node.port = Node4_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 5;
// 	node.ip = Node5_IP;
// 	node.port = Node5_Port;
// 	grid.AddNode(node);
// 	node.nodeId = 6;
// 	node.ip = Node6_IP;
// 	node.port = Node6_Port;
// 	grid.AddNode(node);
	grid.InitClientId();

	std::vector<Grid::Point> data;
	std::vector<bigint> inIds;
	std::vector<Grid::SELECT_LINE> selectLines;
	Grid::SELECT_LINE selectLine;
	std::vector<Grid::FILTER> pointFilter;
	std::vector<std::string> getFields;

	Grid::FILTER filter;
	getFields.push_back("coin");
	getFields.push_back("name");

	CallResult ret;
	bigint p0 = points[0];
	bigint p1 = points[1];
	while ( true )
	{
		inIds.clear();
		inIds.push_back(p0);
		inIds.push_back(p1);

		selectLines.clear();
		filter.fieldName = "关系id";
		filter.field.type = Grid::int32;
		filter.field.value = LinkType::Friend;
		filter.cmpMode = Grid::equals;
		selectLine.moveOut = true;
		selectLine.moveAllLine = false;
		selectLine.selectField.clear();
		selectLine.selectField.push_back(filter);
		selectLines.push_back(selectLine);

		filter.fieldName = "关系id";
		filter.field.type = Grid::int32;
		filter.field.value = LinkType::Schoolmate;
		filter.cmpMode = 0;
		selectLine.moveOut = true;
		selectLine.moveAllLine = false;
		selectLine.selectField.clear();
		selectLine.selectField.push_back(filter);
// 		selectLines.push_back(selectLine);
		mdk::uint64 start = mdk::MillTime();
		ret = grid.Move(data, inIds, selectLines, pointFilter, true, getFields, Grid::get);
		if ( !ret.isSuccess )
		{
			printf( "失败：%s\n", ret.reason.c_str() );
		}
		else
		{
			printf( "结果数量：%d\n", data.size() );
		}
		start = mdk::MillTime() - start;
		int i = 0;
		for ( i = 0; i < data.size(); i++ )
		{
			data[i].Release();
		}
		data.clear();
	}
	return;
}

void TestLine(std::vector<bigint> &points)
{
	GCluster grid;
	NODE node;
	node.nodeId = 1;
	node.ip = Node1_IP;
	node.port = Node1_Port;
	grid.AddNode(node);

	Grid::Line line;
	Grid::FIELD *pCoin = new Grid::FIELD;
	line.data.insert("coin", strlen("coin"), pCoin);
	pCoin->type = Grid::int32;
	Grid::FIELD *pName = new Grid::FIELD;
	line.data.insert("name", strlen("name"), pName);
	pName->type = Grid::str;
	Grid::FIELD *pId = new Grid::FIELD;
	line.data.insert("id", strlen("id"), pId);
	pId->type = Grid::int32;
	char str[256];
	int i = 0;
	CallResult ret;
	std::vector<bigint> lines;
	Grid::Point replyPoint;
	Grid::Line replyLine;
	for ( i = 0; true; i++ )
	{
		line.id = 0;
		line.startId = 1;
		line.endId = 2;
		sprintf(str, "line%d", i + 1);
// 		pName->size = strlen(str) + 1;
		pId->value = i + 1;
		pCoin->value = i;
		ret = grid.Link(line);
		if ( !ret.isSuccess )
		{
			printf("%s\n", ret.reason.c_str());
		}
		ret = grid.GetLine(line.id, replyLine);
		if ( !ret.isSuccess )
		{
			printf("%s\n", ret.reason.c_str());
		}
		replyLine.Release();
		bigint startId, endId;
		ret = grid.DelLine(line.id, startId, endId);
		if ( !ret.isSuccess )
		{
			printf("%s\n", ret.reason.c_str());
		}
		ret = grid.GetLine(line.id, replyLine);
		if ( ret.isSuccess || "无数据" != ret.reason)
		{
			printf("%s\n", ret.reason.c_str());
		}
// 		ret = grid.Link(line);
// 		if ( !ret.isSuccess )
// 		{
// 			printf("%s\n", ret.reason.c_str());
// 		}
		if ( false )
		{//修改顶点的边测试
// 			ret = grid.GetPoint(line.startId, replyPoint);
// 			if ( !ret.isSuccess )
// 			{
// 				printf("%s\n", ret.reason.c_str());
// 			}
// 			replyPoint.Release();
// 			ret = grid.DelLineForPoint(line.startId, line.id);
// 			if ( !ret.isSuccess )
// 			{
// 				printf("%s\n", ret.reason.c_str());
// 			}
// 			ret = grid.GetPoint(line.startId, replyPoint);
// 			if ( !ret.isSuccess )
// 			{
// 				printf("%s\n", ret.reason.c_str());
// 			}
// 			replyPoint.Release();
// 			ret = grid.AddLineForPoint(line.startId, line.id);
// 			if ( !ret.isSuccess )
// 			{
// 				printf("%s\n", ret.reason.c_str());
// 			}
// 			ret = grid.GetPoint(line.startId, replyPoint);
// 			if ( !ret.isSuccess )
// 			{
// 				printf("%s\n", ret.reason.c_str());
// 			}
// 			replyPoint.Release();
		}
		if ( false )
		{//unlink测试
// 			grid.Unlink(line.id);
// 			ret = grid.GetLine(line.id, replyLine);
// 			if ( ret.isSuccess || "无数据" != ret.reason)
// 			{
// 				printf("边未断开成功\n");
// 				replyLine.Release();
// 			}
// 			ret = grid.GetPoint(line.startId, replyPoint);
// 			if ( !ret.isSuccess )
// 			{
// 				printf("%s\n", ret.reason.c_str());
// 			}
// 			replyPoint.Release();
// 			ret = grid.GetPoint(line.endId, replyPoint);
// 			if ( !ret.isSuccess )
// 			{
// 				printf("%s\n", ret.reason.c_str());
// 			}
// 			replyPoint.Release();
		}
		if ( false )
		{//DelPoint测试
// 			ret = grid.DelPoint(line.endId);
// 			if ( !ret.isSuccess )
// 			{
// 				printf("%s\n", ret.reason.c_str());
// 			}
// 			ret = grid.GetPoint(line.startId, replyPoint);
// 			if ( !ret.isSuccess )
// 			{
// 				printf("%s\n", ret.reason.c_str());
// 			}
// 			replyPoint.Release();
// 			ret = grid.GetLine(line.id, replyLine);
// 			if ( ret.isSuccess || "无数据" != ret.reason)
// 			{
// 				printf("边未删除成功\n");
// 				replyLine.Release();
// 			}
// 			ret = grid.GetPoint(line.endId, replyPoint);
// 			if ( ret.isSuccess || "无数据" != ret.reason)
// 			{
// 				printf("终点未删除成功\n");
// 				replyPoint.Release();
// 			}
		}
	}
}

int main(int argc, char* argv[])
{
	TestPoint();
	return 0;
}
