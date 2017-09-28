#ifndef G_CLUSTER_H
#define G_CLUSTER_H

#include "GMap.h"
#include <map>
#include <vector>
#include "common/Common.h"
#include "../gsrc/GSrc.h"

//图数据库集群
class GCluster
{
public:
	GCluster();
	virtual ~GCluster();

	bool AddNode( NODE &node );//添加节点
	bool AddNodes(std::vector<NODE> &nodes);//添加节点
	void CloseCluster();//断开集群连接

	CallResult InitClientId();//初始化client唯一id
	//////////////////////////////////////////////////////////////////////////
	//原子操作
	CallResult NewId(bigint &id);//产生一个唯一id
	CallResult SetPoint(Grid::Point &point);//设置点
	CallResult AddLineForPoint(bigint pointId, bigint lineId, bool isOut);//给顶点增加边
	CallResult DelLineForPoint(bigint pointId, bigint lineId, bool isOut);//删除顶点上的边
	/*
		取得顶点
		为了避免内存创建、copy、销毁造成的性能开销
		直接使用msg自带缓存，没有使用Grid::Line做参数
	*/
	CallResult GetPoint(bigint pointId, Grid::Point &point);
	/*
		取得边
		为了避免内存创建、copy、销毁造成的性能开销
		直接使用msg自带缓存，没有使用Grid::Line做参数
	*/
	CallResult GetLine(bigint lineId, Grid::Line &line);
	/*
		删除边，取得断开前连接的2个顶点
		只删除边数据，不删除与边连接的2个顶点上的边
	*/
	CallResult DelLine(bigint lineId, bigint &startId, bigint &endId);

	//////////////////////////////////////////////////////////////////////////
	//数据库基础操作
	CallResult Link(Grid::Line &line);//连接顶点
	CallResult DelPoint(bigint pointId);//删除顶点
	CallResult Unlink(bigint lineId);//断开连接,并修改顶点
	//////////////////////////////////////////////////////////////////////////
	//查询操作
	//从顶点移动，取得可到达的所有顶点
	CallResult Move(std::vector<Grid::Point> &points, 
		std::vector<bigint> &ids, 
		std::vector<Grid::SELECT_LINE> &selectLines, std::vector<Grid::FILTER> &pointFilter, 
		bool selectAll, std::vector<std::string> &getFields, Grid::Action act);

protected:
	CallResult Move(std::vector<Grid::Point> &points, std::map<bigint, bool> &repeat, 
		std::vector<bigint> &ids, bool fromPoint, bool filterStartPoint, std::map<int, Grid::REMOTE_DATA> &movesParam,
		std::vector<Grid::SELECT_LINE> &selectLines, std::vector<Grid::FILTER> &pointFilter, 
		bool selectAll, std::vector<std::string> &getFields, Grid::Action act);
	//////////////////////////////////////////////////////////////////////////
	//路由操作
	GMap& NextNode();//轮训下一个节点
	GMap& FindNode(bigint dataId);//找到数据归属节点

private:
	GSrc	m_gsrc;//集群配置
	std::map<int, GMap>			m_nodes;
	std::map<int, GMap>::iterator	m_itNextNode;//下一个节点
	bigint	m_clientId;//唯一id，用于区分搜索，每个线程1个client id
	bigint	m_searchId;
	int		m_searchIndex;//搜索顺序号
};

#endif //G_CLUSTER_H