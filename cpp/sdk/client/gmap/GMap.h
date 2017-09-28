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

	CallResult NewId(bigint &id);//����һ��Ψһid
	CallResult NewIds(std::vector<bigint> &ids, int count);//����count��Ψһid
	CallResult SetPoint(Grid::Point &point);//���õ�
	CallResult GetPoint(bigint pointId, Grid::Point &point);//ȡ�ö���
	CallResult AddLine(Grid::Line &line);//������
	CallResult DelPoint(bigint pointId);//ɾ������

	CallResult AddLineForPoint(bigint pointId, bigint lineId, bool isOut);//���������ӱ�
	CallResult DelLineForPoint(bigint pointId, bigint lineId, bool isOut);//ɾ�������ϵı�
	CallResult DelLine(bigint lineId, bigint &startId, bigint &endId);//ɾ����
	CallResult GetLine(bigint lineId, Grid::Line &line);//ȡ�ñ�

	//////////////////////////////////////////////////////////////////////////
	//ͼ�ж�������
	CallResult SetMoveOpt(std::vector<Grid::SELECT_LINE> &selectLines, std::vector<Grid::FILTER> &pointFilter, 
		bool selectAll, std::vector<std::string> &getFields, Grid::Action &act);
	CallResult Move(bigint searchId, std::vector<bigint> &ids, bool fromPoint, bool filterStartPoint);

	CallResult ReadMsg(msg::Buffer *msg, int millSecond);
};

#endif //GMAP_H