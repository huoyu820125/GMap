#ifndef G_CLUSTER_H
#define G_CLUSTER_H

#include "GMap.h"
#include <map>
#include <vector>
#include "common/Common.h"
#include "../gsrc/GSrc.h"

//ͼ���ݿ⼯Ⱥ
class GCluster
{
public:
	GCluster();
	virtual ~GCluster();

	bool AddNode( NODE &node );//��ӽڵ�
	bool AddNodes(std::vector<NODE> &nodes);//��ӽڵ�
	void CloseCluster();//�Ͽ���Ⱥ����

	CallResult InitClientId();//��ʼ��clientΨһid
	//////////////////////////////////////////////////////////////////////////
	//ԭ�Ӳ���
	CallResult NewId(bigint &id);//����һ��Ψһid
	CallResult SetPoint(Grid::Point &point);//���õ�
	CallResult AddLineForPoint(bigint pointId, bigint lineId, bool isOut);//���������ӱ�
	CallResult DelLineForPoint(bigint pointId, bigint lineId, bool isOut);//ɾ�������ϵı�
	/*
		ȡ�ö���
		Ϊ�˱����ڴ洴����copy��������ɵ����ܿ���
		ֱ��ʹ��msg�Դ����棬û��ʹ��Grid::Line������
	*/
	CallResult GetPoint(bigint pointId, Grid::Point &point);
	/*
		ȡ�ñ�
		Ϊ�˱����ڴ洴����copy��������ɵ����ܿ���
		ֱ��ʹ��msg�Դ����棬û��ʹ��Grid::Line������
	*/
	CallResult GetLine(bigint lineId, Grid::Line &line);
	/*
		ɾ���ߣ�ȡ�öϿ�ǰ���ӵ�2������
		ֻɾ�������ݣ���ɾ��������ӵ�2�������ϵı�
	*/
	CallResult DelLine(bigint lineId, bigint &startId, bigint &endId);

	//////////////////////////////////////////////////////////////////////////
	//���ݿ��������
	CallResult Link(Grid::Line &line);//���Ӷ���
	CallResult DelPoint(bigint pointId);//ɾ������
	CallResult Unlink(bigint lineId);//�Ͽ�����,���޸Ķ���
	//////////////////////////////////////////////////////////////////////////
	//��ѯ����
	//�Ӷ����ƶ���ȡ�ÿɵ�������ж���
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
	//·�ɲ���
	GMap& NextNode();//��ѵ��һ���ڵ�
	GMap& FindNode(bigint dataId);//�ҵ����ݹ����ڵ�

private:
	GSrc	m_gsrc;//��Ⱥ����
	std::map<int, GMap>			m_nodes;
	std::map<int, GMap>::iterator	m_itNextNode;//��һ���ڵ�
	bigint	m_clientId;//Ψһid����������������ÿ���߳�1��client id
	bigint	m_searchId;
	int		m_searchIndex;//����˳���
};

#endif //G_CLUSTER_H