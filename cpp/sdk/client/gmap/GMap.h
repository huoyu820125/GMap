#ifndef GMAP_H
#define GMAP_H

#include "Protocl/cpp/base/Socket.h"
#include "Protocl/cpp/Buffer.h"
#include <string>
#include <map>
#include <cstdlib>
#include <cstdio>

#include "common/CallResult.h"
#include "../Client.h"
#include "common/Grid.h"
#include "common/struct/Point.h"
#include "common/struct/Line.h"


class GMap : public Client
{
public:
	GMap();
	virtual ~GMap();

	CallResult NewId(bigint &id);//产生一个唯一id
	CallResult NewIds(std::vector<bigint> &ids, int count);//产生count个唯一id
	CallResult SetPoint(Grid::Point &point);//设置点
	CallResult GetPoint(bigint pointId, Grid::Point &point);//取得顶点
	CallResult AddLine(Grid::Line &line);//创建边
	CallResult DelPoint(bigint pointId);//删除顶点

	CallResult AddLineForPoint(bigint pointId, bigint lineId, bool isOut);//给顶点增加边
	CallResult DelLineForPoint(bigint pointId, bigint lineId, bool isOut);//删除顶点上的边
	CallResult DelLine(bigint lineId, bigint &startId, bigint &endId);//删除边
	CallResult GetLine(bigint lineId, Grid::Line &line);//取得边

	//////////////////////////////////////////////////////////////////////////
	//图中顶点游走
	CallResult SetMoveOpt(std::vector<Grid::SELECT_LINE> &selectLines, std::vector<Grid::FILTER> &pointFilter, 
		bool selectAll, std::vector<std::string> &getFields, Grid::Action &act);
	CallResult Move(bigint searchId, std::vector<bigint> &ids, bool fromPoint, bool filterStartPoint);

	CallResult ReadMsg(msg::Buffer *msg, int millSecond);
};

#endif //GMAP_H