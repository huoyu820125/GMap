#ifndef GRIDSTORE_H
#define GRIDSTORE_H
#include "Points.h"
#include "Lines.h"
#include <vector>
#include <map>
#include "common/CallResult.h"
#include "DiskIO.h"
#include "mdk_ex/container/Array.h"
#include "mdk_ex/RHTable.h"


//图数据库存储节点
//重要提示，本类中的方法不可加lock，否则造成分布式死锁

class Worker;
class GridStore
{
public:
	GridStore();
	virtual ~GridStore();
	const char* Init(int nodeId, int nodeCount);//成功返回null，失败返回原因
	void SetWorker(Worker *pWorker);
	Worker* GetWorker();
	DiskIO* GetDisk();
	int NodeId();

	//////////////////////////////////////////////////////////////////////////
	//原子操作
	Grid::Result::Result CreateId(bigint &id, int count);//创建id，分布式唯一
	CallResult SetPoint(Grid::Point &point);//设置顶点，id>0设置顶点,id<=0创建顶点
	CallResult AddLineForPoint(bigint pointId, bigint lineId, bool isOut);//给顶点增加边
	CallResult DelLineForPoint(bigint pointId, bigint lineId, bool isOut);//删除顶点上的边
	CallResult GetPoint(Grid::Point* &pPoint, bigint pointId);//取得顶点
	CallResult AddLine(Grid::Line &line);//创建边
	CallResult DelLine(bigint lineId, bigint &startId, bigint &endId);//删除边，取得边之前连接的顶点
	CallResult GetLine(Grid::Line* &pLine, bigint lineId);//取得边

	//////////////////////////////////////////////////////////////////////////
	//基础操作
	CallResult DelPoint(bigint pointId);//删除顶点

	//////////////////////////////////////////////////////////////////////////
	//move查询
	//移动多次
	CallResult Move(bigint &searchNo, mdk::Array<Grid::Point*> &endPoints, std::vector<Grid::REMOTE_DATA> &remoteDatas,
		bool isPointId, bool filterStartPoint, std::vector<bigint> &ids, 
		std::vector<Grid::SELECT_LINE> &selectLines, 
		std::vector<Grid::FILTER> &pointFilter, Grid::Action act);

	//批量创建顶点
	CallResult CreatePoints(std::vector<Grid::Point> &points);

private:
 	/*
		从顶点开始移动
		从一组起点开始移动，按照selectLines选择路线，到达的顶点
		按照pointFilter过滤达到的顶点，执行act操作。

		points			返回得到的顶点
		lineIds			不在本地的边id列表，下一步执行Move1
		pointIds		不在本地的顶点id列表，下一步执行FilterPoint

		startPointIds	起点Id列表

		out				路线是出边
		moveAllLine		从out指定方向的所有路线移动 moveAllLine = true时，selectLines无效
		selectLines		移动路线，按照属性选择路线
		pointFilter		按照属性选择顶点
		act				执行的操作有get，count，del操作
		getFields		act = get时指定获取顶点哪些属性，id必然获取，传递size=0的数组，表示取所有属性
	*/
	CallResult MoveStart(mdk::Array<Grid::Point*> &points, 
		std::vector<bigint> &lineIds, std::vector<bigint> &pointIds,
		std::vector<bigint> &startPointIds, bool out, 
		bool moveAllLines, std::vector<Grid::FILTER> &selectLines, 
		std::vector<Grid::FILTER> &pointFilter, 
		Grid::Action act);
	/*
		从边开始移动
		按照selectLines在lineIds中选择路线，移动到顶点
		按照pointFilter过了到达的顶点，执行act操作。

		points			返回得到的顶点
		pointIds		不在本地的顶点id列表，下一步执行FilterPoint

		lineIds			开始的路线
		out				路线是出边
		moveAllLine		从out指定方向的所有路线移动 moveAllLine = true时，selectLines无效
		selectLines		路线过滤，按照属性选择路线
		pointFilter		按照属性选择顶点
		act				执行的操作有get，count，del操作
	*/
	CallResult MoveLine(mdk::Array<Grid::Point*> &points,
		std::vector<bigint> &pointIds, 
		std::vector<bigint> &lineIds, bool out, 
		bool moveAllLines, std::vector<Grid::FILTER> &selectLines, 
		std::vector<Grid::FILTER> &pointFilter, 
		Grid::Action act);
	//过滤顶点
	CallResult FilterPoint(mdk::Array<Grid::Point*> &points, 
		std::vector<bigint> &ids, 
		std::vector<Grid::FILTER> &pointFilter, Grid::Action act);
	//比较过滤条件,判断数据是否被选择
	bool IsSelected(mdk::Map &data, std::vector<Grid::FILTER> &filter);


	//查找节点id
	int FindNodeId(bigint dataId);
private:
	Grid::Result::Result GetSuperId( int &superId );

	bool IsOwner(bigint dataId);//是数据的拥有者（数据存储在本节点上）
	CallResult ToMsgResult(Grid::Result::Result result);//api结果转换成api结果
	bool LoadLine(Grid::Line *pLine, bool loadStartPooint);//加载本地起点终点，如果加载失败，并清理数据
	bool LoadPoint(Grid::Point *pPoint, bool out, int pos);//加载边，如果加载失败，并清理数据

private:
	DiskIO	m_disk;//硬盘
	int		m_superId;//超级id,用于生成唯一id
	int		m_nodeId;//结点编号
	int		m_nodeCount;//节点数量
	Points	m_storePoint;//顶点数据库
	Lines	m_storeLine;//边数据库
	Worker	*m_pWorker;//工作者
	bigint	m_searchNo;//检索号，每次检索生成1个新的
	time_t	m_searchTime;//搜索开始时间
	bool	m_isEndPoint;//检索结果是最终顶点
};

#endif //GRIDSTORE_H
