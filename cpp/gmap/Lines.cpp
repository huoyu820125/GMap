#include "Lines.h"
#include "GridStore.h"
#include "mdk/include/mdk/atom.h"
#include "common/struct/Point.h"
#include "common/heap.h"


Lines::Lines(GridStore *pStore)
{
	m_pStore = pStore;
	m_pDisk = m_pStore->GetDisk();
}


Lines::~Lines()
{
}

const char* Lines::LoadData(int &count)
{
	const char *ret = m_pDisk->LoadLines(m_lines);
	count = m_lines.size();
	return ret;
}

//使用直线line链接2个顶点startPoint和endPoint
Grid::Result::Result Lines::CreateLine(Grid::Line &line, Grid::Point *pStartPoint, Grid::Point *pEndPoint)
{
	Grid::Line *pLine = Grid::Line::Alloc();
	pLine->id = line.id;
	pLine->startId = line.startId;
	pLine->endId = line.endId;
	pLine->pStartPoint = pStartPoint;
	pLine->pEndPoint = pEndPoint;
	pLine->diskSpace = 0;
	pLine->SetIdxPos(-1);
	pLine->SetDataPos(-1);
	m_lines[pLine->id] = pLine;

	pLine->UpdateFields(&line);//修改属性
	m_pDisk->SaveLine(*pLine);//修改数据库

	return Grid::Result::success;
}

Grid::Line* Lines::FindLine(bigint lineId)
{
	std::map<bigint, Grid::Line*>::iterator it = m_lines.find(lineId);
	if ( it != m_lines.end() ) return it->second;
	return NULL;
}

Grid::Result::Result Lines::DelLine(bigint lineId, bigint &startId, bigint &endId)
{
	std::map<bigint, Grid::Line*>::iterator it = m_lines.find(lineId);
	if ( it == m_lines.end() ) return Grid::Result::noData;

	Grid::Line *pLine = it->second;
	startId = pLine->startId;
	endId = pLine->endId;
	bigint dataPos = pLine->DataPos();
	m_lines.erase(it);
	Grid::Line::Free(pLine);

	//写数据库
	m_pDisk->DelLine(lineId, dataPos);

	return Grid::Result::success;
}
