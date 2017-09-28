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


//ͼ���ݿ�洢�ڵ�
//��Ҫ��ʾ�������еķ������ɼ�lock��������ɷֲ�ʽ����

class Worker;
class GridStore
{
public:
	GridStore();
	virtual ~GridStore();
	const char* Init(int nodeId, int nodeCount);//�ɹ�����null��ʧ�ܷ���ԭ��
	void SetWorker(Worker *pWorker);
	Worker* GetWorker();
	DiskIO* GetDisk();
	int NodeId();

	//////////////////////////////////////////////////////////////////////////
	//ԭ�Ӳ���
	Grid::Result::Result CreateId(bigint &id, int count);//����id���ֲ�ʽΨһ
	CallResult SetPoint(Grid::Point &point);//���ö��㣬id>0���ö���,id<=0��������
	CallResult AddLineForPoint(bigint pointId, bigint lineId, bool isOut);//���������ӱ�
	CallResult DelLineForPoint(bigint pointId, bigint lineId, bool isOut);//ɾ�������ϵı�
	CallResult GetPoint(Grid::Point* &pPoint, bigint pointId);//ȡ�ö���
	CallResult AddLine(Grid::Line &line);//������
	CallResult DelLine(bigint lineId, bigint &startId, bigint &endId);//ɾ���ߣ�ȡ�ñ�֮ǰ���ӵĶ���
	CallResult GetLine(Grid::Line* &pLine, bigint lineId);//ȡ�ñ�

	//////////////////////////////////////////////////////////////////////////
	//��������
	CallResult DelPoint(bigint pointId);//ɾ������

	//////////////////////////////////////////////////////////////////////////
	//move��ѯ
	//�ƶ����
	CallResult Move(bigint &searchNo, mdk::Array<Grid::Point*> &endPoints, std::vector<Grid::REMOTE_DATA> &remoteDatas,
		bool isPointId, bool filterStartPoint, std::vector<bigint> &ids, 
		std::vector<Grid::SELECT_LINE> &selectLines, 
		std::vector<Grid::FILTER> &pointFilter, Grid::Action act);

	//������������
	CallResult CreatePoints(std::vector<Grid::Point> &points);

private:
 	/*
		�Ӷ��㿪ʼ�ƶ�
		��һ����㿪ʼ�ƶ�������selectLinesѡ��·�ߣ�����Ķ���
		����pointFilter���˴ﵽ�Ķ��㣬ִ��act������

		points			���صõ��Ķ���
		lineIds			���ڱ��صı�id�б���һ��ִ��Move1
		pointIds		���ڱ��صĶ���id�б���һ��ִ��FilterPoint

		startPointIds	���Id�б�

		out				·���ǳ���
		moveAllLine		��outָ�����������·���ƶ� moveAllLine = trueʱ��selectLines��Ч
		selectLines		�ƶ�·�ߣ���������ѡ��·��
		pointFilter		��������ѡ�񶥵�
		act				ִ�еĲ�����get��count��del����
		getFields		act = getʱָ����ȡ������Щ���ԣ�id��Ȼ��ȡ������size=0�����飬��ʾȡ��������
	*/
	CallResult MoveStart(mdk::Array<Grid::Point*> &points, 
		std::vector<bigint> &lineIds, std::vector<bigint> &pointIds,
		std::vector<bigint> &startPointIds, bool out, 
		bool moveAllLines, std::vector<Grid::FILTER> &selectLines, 
		std::vector<Grid::FILTER> &pointFilter, 
		Grid::Action act);
	/*
		�ӱ߿�ʼ�ƶ�
		����selectLines��lineIds��ѡ��·�ߣ��ƶ�������
		����pointFilter���˵���Ķ��㣬ִ��act������

		points			���صõ��Ķ���
		pointIds		���ڱ��صĶ���id�б���һ��ִ��FilterPoint

		lineIds			��ʼ��·��
		out				·���ǳ���
		moveAllLine		��outָ�����������·���ƶ� moveAllLine = trueʱ��selectLines��Ч
		selectLines		·�߹��ˣ���������ѡ��·��
		pointFilter		��������ѡ�񶥵�
		act				ִ�еĲ�����get��count��del����
	*/
	CallResult MoveLine(mdk::Array<Grid::Point*> &points,
		std::vector<bigint> &pointIds, 
		std::vector<bigint> &lineIds, bool out, 
		bool moveAllLines, std::vector<Grid::FILTER> &selectLines, 
		std::vector<Grid::FILTER> &pointFilter, 
		Grid::Action act);
	//���˶���
	CallResult FilterPoint(mdk::Array<Grid::Point*> &points, 
		std::vector<bigint> &ids, 
		std::vector<Grid::FILTER> &pointFilter, Grid::Action act);
	//�ȽϹ�������,�ж������Ƿ�ѡ��
	bool IsSelected(mdk::Map &data, std::vector<Grid::FILTER> &filter);


	//���ҽڵ�id
	int FindNodeId(bigint dataId);
private:
	Grid::Result::Result GetSuperId( int &superId );

	bool IsOwner(bigint dataId);//�����ݵ�ӵ���ߣ����ݴ洢�ڱ��ڵ��ϣ�
	CallResult ToMsgResult(Grid::Result::Result result);//api���ת����api���
	bool LoadLine(Grid::Line *pLine, bool loadStartPooint);//���ر�������յ㣬�������ʧ�ܣ�����������
	bool LoadPoint(Grid::Point *pPoint, bool out, int pos);//���رߣ��������ʧ�ܣ�����������

private:
	DiskIO	m_disk;//Ӳ��
	int		m_superId;//����id,��������Ψһid
	int		m_nodeId;//�����
	int		m_nodeCount;//�ڵ�����
	Points	m_storePoint;//�������ݿ�
	Lines	m_storeLine;//�����ݿ�
	Worker	*m_pWorker;//������
	bigint	m_searchNo;//�����ţ�ÿ�μ�������1���µ�
	time_t	m_searchTime;//������ʼʱ��
	bool	m_isEndPoint;//������������ն���
};

#endif //GRIDSTORE_H
