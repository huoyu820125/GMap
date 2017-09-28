#include "Points.h"
#include "GridStore.h"
#include "Worker.h"
#include "mdk/include/mdk/atom.h"
#include "common/heap.h"


Points::Points(GridStore *pStore)
{
	m_pStore = pStore;
	m_pDisk = m_pStore->GetDisk();
}

Points::~Points()
{
}

const char* Points::LoadData(int &count)
{
	const char* ret = m_pDisk->LoadPoints(m_points);
	count = m_points.size();
	return ret;
}

Grid::Result::Result Points::SetPoint(Grid::Point &point)
{
	Grid::Point *pPoint = FindPoint(point.id);
	if ( NULL == pPoint )//�½�����
	{
  		pPoint = Grid::Point::Alloc();
 		pPoint->id = point.id;
		pPoint->searchNo = -1;//���1�μ�����=null
		pPoint->diskSpace = 0;
		pPoint->SetIdxPos(-1);
		pPoint->SetDataPos(-1);
		m_points[point.id] = pPoint;
	}
	if ( pPoint == &point ) return Grid::Result::success;

	//�޸�����
	pPoint->UpdateFields(&point);
	//�޸����ݿ�
	return m_pDisk->SavePoint(*pPoint);
}

Grid::Point* Points::FindPoint(bigint id)
{
	std::map<bigint, Grid::Point*>::iterator it = m_points.find(id);
	if ( it != m_points.end() ) return it->second;
	return NULL;
}

Grid::Point* Points::AddLine(bigint pointId, bigint lineId, bool isOut)
{
	Grid::Point *pPoint = FindPoint(pointId);
	if ( NULL == pPoint ) 
	{
		return NULL;
	}

	if ( !pPoint->AddLine(lineId, isOut) ) 
	{
		return pPoint;
	}

	//�޸����ݿ�
	if ( Grid::Result::success != m_pDisk->SavePoint(*pPoint) ) 
	{
		return NULL;
	}

	return pPoint;
}

Grid::Point* Points::DelLine(bigint pointId, bigint lineId, bool isOut)
{
	Grid::Point *pPoint = FindPoint(pointId);
	if ( NULL == pPoint ) return NULL;

	std::map<bigint, void*>::iterator it;
	
	if ( !pPoint->DelLine(lineId, isOut) ) return pPoint;

	//�޸����ݿ�
	if ( Grid::Result::success != m_pDisk->SavePoint(*pPoint) ) return NULL;

	return pPoint;
}

Grid::Result::Result Points::DelPoint(bigint pointId)
{
	std::map<bigint, Grid::Point*>::iterator it = m_points.find(pointId);
	if ( it == m_points.end() ) return Grid::Result::success;

	Grid::Point *pPoint = it->second;
	bigint dataPos = pPoint->DataPos();

	//�޸����ݿ�
	Grid::Result::Result ret = m_pDisk->DelPoint(pointId, dataPos);
	if ( Grid::Result::dataDamage == ret || Grid::Result::noDiskSpace == ret ) return ret;
	//���ݿ��޸ĳɹ����Ŵ��ڴ��ɾ��
	m_points.erase(it);
	Grid::Point::Free(pPoint);

	return Grid::Result::success;
}

//���涥������
bool Points::SavePoint(Grid::Point *pPoint)
{
	if ( Grid::Result::success != m_pDisk->SavePoint(*pPoint) ) return false;
	return true;
}

Grid::Result::Result Points::CreatePoints(std::vector<Grid::Point> &points)
{
	if ( 0 == points.size() ) return Grid::Result::paramError;
	Grid::Result::Result ret;
	std::vector<Grid::Point*> newPoints;
	Grid::Point *pPoint = NULL;
	int i = 0;
	int pos = INDEX_SIZE;
	for ( ; i < points.size(); i++ )
	{
		if ( 0 >= points[i].id ) return Grid::Result::paramError;
		pPoint = FindPoint(points[i].id);
		if ( NULL != pPoint ) return Grid::Result::paramError;

		pPoint = Grid::Point::Alloc();
		pos += INDEX_SIZE + sizeof(Grid::Point);
		pPoint->id = points[i].id;
		pPoint->searchNo = -1;//���1�μ�����=null
		pPoint->diskSpace = 0;
		m_points[pPoint->id] = pPoint;
		newPoints.push_back(pPoint);
	}
	return m_pDisk->CreatePoints(newPoints);
}

