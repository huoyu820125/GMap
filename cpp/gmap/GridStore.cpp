#include "GridStore.h"
#include "mdk/include/mdk/Lock.h"
#include "mdk/include/mdk/mapi.h"
#include "mdk/include/mdk/atom.h"
#include "mdk_ex/File.h"
#include "Worker.h"
#include <ctime>

//��Ҫ��ʾ�������еķ������ɼ�lock��������ɷֲ�ʽ����
GridStore::GridStore()
:m_storePoint(this), m_storeLine(this)
{
	m_superId = 0;
	m_pWorker = NULL;
}

GridStore::~GridStore()
{
}

void GridStore::SetWorker(Worker *pWorker)
{
	m_pWorker = pWorker;
}

Worker* GridStore::GetWorker()
{
	return m_pWorker;
}

DiskIO* GridStore::GetDisk()
{
	return &m_disk;
}

int GridStore::NodeId()
{
	return m_nodeId;
}

const char* GridStore::Init(int nodeId, int nodeCount)
{
	m_nodeId = nodeId;
	m_nodeCount = nodeCount;
	if ( m_nodeId > 1000 ) return "�ڵ��������ܳ���1000";

	m_disk.Init(this);
	m_pWorker->Logger().Info("run", "����Ŀ¼��%s", m_disk.DataDir().c_str());
	const char *reason = m_disk.CheckDataDir();
	if ( NULL != reason ) return reason;

	Grid::Result::Result ret = GetSuperId(m_superId);
	if ( Grid::Result::success != ret ) return "���ܳ�ʼ��Ψһid";

	int pcount;
	reason = m_storePoint.LoadData(pcount);
	if ( NULL != reason ) return reason;
	m_pWorker->Logger().Info("run", "���ݿ���У�%d������", pcount);
	reason = m_storeLine.LoadData(pcount);
	m_pWorker->Logger().Info("run", "���ݿ���У�%d����", pcount);

	return reason;
}

//ȡ�ó���id
Grid::Result::Result GridStore::GetSuperId( int &superId )
{
	//10b(nodeId:0~1023) + 16b(superId��0~65535) + 20b(index��0~1048575)
	char exeDir[256];
	mdk::GetExeDir( exeDir, 256 );//ȡ�ÿ�ִ�г���λ��
	char fileName[256];
	sprintf( fileName, "node%d/superId", m_nodeId );
	mdk::File file(exeDir, fileName);
	mdk::File::Error ret = file.Open(mdk::File::read, mdk::File::assii);
	int nextId;
	if ( mdk::File::success != ret ) 
	{
		if ( mdk::File::fileNoExist != ret ) return Grid::Result::fileError;
		superId = 0;//�ļ������ڣ���һ�δ�����0��ʼ
	}
	else
	{
		if ( mdk::File::success != file.Read(&superId, sizeof(int)) ) return Grid::Result::fileError;
		if ( !file.Close() ) return Grid::Result::fileError;
	}
	if ( 65535 < superId ) return Grid::Result::noIdSource;
	nextId = superId + 1;
	m_superId += m_nodeId << 16;
	if ( mdk::File::success != file.Open(mdk::File::write, mdk::File::assii) ) return Grid::Result::fileError;
	if ( mdk::File::success != file.Write(&nextId, sizeof(int)) ) return Grid::Result::fileError;
	if ( !file.Close() ) return Grid::Result::fileError;

	return Grid::Result::success;
}

Grid::Result::Result GridStore::CreateId(bigint &id, int count)
{
	if ( 0 >= count || Grid::Limit::maxObjectCount < count ) return Grid::Result::paramError;

	//10b(nodeId:0~1024) + 16b(superId��0~65535) + 20b(index��0~1048575)
	static int index = 0;
	if ( 1048575 < index ) //ȷ��һ�����õ����
	{
		index = 0;
		Grid::Result::Result ret = GetSuperId(m_superId);
		if ( Grid::Result::success != ret ) return ret;
	}
	//��¼��ʼid
	id = m_superId;
	id = id << 20;
	id += index;
	//˳�������count��
	int useCount = 0;
	while ( 0 < count )//��Ҫ��������id
	{
		if ( 1048575 < index ) 
		{
			index = 0;
			Grid::Result::Result ret = GetSuperId(m_superId);
			if ( Grid::Result::success != ret ) return ret;
		}

		useCount = 1048575 - index + 1;//����id������ȷ��index������Խ�絽��һ���ֶ�
		if ( useCount >= count ) useCount = count;//����ʹ������
		index += useCount;//������һ��δʹ�õ����
		count -= useCount;
	}

	return Grid::Result::success;
}

//�����ݵ�ӵ���ߣ����ݴ洢�ڱ��ڵ��ϣ�
bool GridStore::IsOwner(bigint dataId)
{
	return Grid::IsOwner(dataId, m_nodeId, m_nodeCount);
}

int GridStore::FindNodeId(bigint dataId)
{
	return Grid::FindNodeId(dataId, m_nodeCount);
}

CallResult GridStore::ToMsgResult(Grid::Result::Result result)
{
	if ( Grid::Result::success == result ) return Call::Success();

	CallResult ret;
	ret.isSuccess = false;
	ret.reason = Grid::Result::Reason(result);
	if ( Grid::Result::paramError == result ) ret.code = ResultCode::paramError;
	else ret.code = ResultCode::refuse;

	return ret;
}

CallResult GridStore::SetPoint(Grid::Point &point)
{
	if ( 0 >= point.id ) return Call::Error(ResultCode::paramError, "����id<=0");
	if ( !IsOwner(point.id) ) return Call::Error(ResultCode::refuse, Reason(Grid::Result::notOwner));

	Grid::Result::Result ret = m_storePoint.SetPoint(point);
	return ToMsgResult(ret);
}

CallResult GridStore::AddLineForPoint(bigint pointId, bigint lineId, bool isOut)
{
	if ( 0 >= pointId ) 
	{
		return Call::Error(ResultCode::paramError, "����id<=0");
	}
	if ( !IsOwner(pointId) ) 
	{
		return Call::Error(ResultCode::refuse, Reason(Grid::Result::notOwner));
	}
	if ( NULL == m_storePoint.AddLine(pointId, lineId, isOut) ) 
	{
		return ToMsgResult(Grid::Result::noData);
	}

	return Call::Success();
}

CallResult GridStore::DelLineForPoint(bigint pointId, bigint lineId, bool isOut)
{
	if ( 0 >= pointId ) return Call::Error(ResultCode::paramError, "����id<=0");
	if ( !IsOwner(pointId) ) return Call::Error(ResultCode::refuse, Reason(Grid::Result::notOwner));
	m_storePoint.DelLine(pointId, lineId, isOut);

	return Call::Success();
}

CallResult GridStore::GetPoint(Grid::Point* &pPoint, bigint pointId)
{
	if ( 0 >= pointId ) 
	{
		m_pWorker->Logger().Info("run", "��ѯ����ʧ�ܣ�����id<=0");
		return Call::Error(ResultCode::paramError, "����id<=0");
	}
	if ( !IsOwner(pointId) ) 
	{
		m_pWorker->Logger().Info("run", "��ѯ����ʧ�ܣ��ڵ㲻������ӵ����");
		return Call::Error(ResultCode::refuse, Reason(Grid::Result::notOwner));
	}

	pPoint = m_storePoint.FindPoint(pointId);
	if ( NULL == pPoint ) 
	{
		CallResult ret;
		ret.isSuccess = false;
		ret.code = ResultCode::refuse;
		ret.reason = Grid::Result::Reason(Grid::Result::noData);
		return ret;
	}

	return Call::Success();
}

CallResult GridStore::AddLine(Grid::Line &line)
{
	if ( 0 >= line.id || 0 >= line.startId || 0 >= line.endId ) return Call::Error(ResultCode::paramError, "����ͱ�id����>0");
	if ( !IsOwner(line.id) ) return Call::Error(ResultCode::refuse, Reason(Grid::Result::notOwner));
	Grid::Result::Result ret;

	//������
	ret = m_storeLine.CreateLine(line, NULL, NULL);

	return ToMsgResult(ret);
}

CallResult GridStore::DelLine(bigint lineId, bigint &startId, bigint &endId)
{
	if ( 0 >= lineId ) return Call::Error(ResultCode::paramError, "��id<=0");
	if ( !IsOwner(lineId) ) return Call::Error(ResultCode::refuse, Reason(Grid::Result::notOwner));

	Grid::Result::Result ret = m_storeLine.DelLine(lineId, startId, endId);
	if ( Grid::Result::noData == ret ) return Call::Success();

	if ( IsOwner(startId) ) 
	{
		DelLineForPoint(startId, lineId, true);
		startId = -1;
	}
	if ( IsOwner(endId) ) 
	{
		DelLineForPoint(endId, lineId, false);
		endId = -1;
	}

	return Call::Success();
}

CallResult GridStore::GetLine(Grid::Line* &pLine, bigint lineId)
{
	if ( 0 >= lineId )
	{
		m_pWorker->Logger().Info("run", "��ѯ��ʧ�ܣ���id<=0");
		return Call::Error(ResultCode::paramError, "��id<=0");
	}
	if ( !IsOwner(lineId) ) 
	{
		m_pWorker->Logger().Info("run", "��ѯ��ʧ�ܣ��ڵ㲻������ӵ����");
		return Call::Error(ResultCode::refuse, Reason(Grid::Result::notOwner));
	}

	pLine = m_storeLine.FindLine(lineId);
	if ( NULL == pLine ) 
	{
		CallResult ret;
		ret.isSuccess = false;
		ret.code = ResultCode::refuse;
		ret.reason = Grid::Result::Reason(Grid::Result::noData);
		return ret;
	}

	return Call::Success();
}

CallResult GridStore::DelPoint(bigint pointId)
{
	if ( 0 >= pointId ) return Call::Error(ResultCode::paramError, "����id<=0");
	if ( !IsOwner(pointId) ) return Call::Error(ResultCode::refuse, Reason(Grid::Result::notOwner));

	//��֤��������Ͳ���ʧ��
	Grid::Point *pPoint = m_storePoint.FindPoint(pointId);
	if ( NULL == pPoint ) return Call::Success();

	//�������б�id
	std::vector<bigint> lines;
	int i = 0;
	for ( i = 0; i < pPoint->out.m_count; i++ ) lines.push_back(pPoint->out.m_list[i].id);
	for ( i = 0; i < pPoint->in.m_count; i++ ) lines.push_back(pPoint->in.m_list[i].id);

	/*
		����ȶϿ����бߣ�Ȼ�󶥵�ɾ��ʧ�ܣ�������ɾ��û�гɹ����������յ�ȴ�����ѯ����
		ֻҪ����ɾ���ɹ������յ㷴��������һ��ʧ�ܣ��������յ�����û���������Ҳһ��ʧ��
		ֻҪ����ɾ��ʧ�ܣ��������������޸ġ�
		���Ա����������������������
	*/
	Grid::Result::Result ret = m_storePoint.DelPoint(pointId);
	if ( Grid::Result::success != ret ) 
	{
		m_pWorker->Logger().Info("Error", "ɾ������ʧ�ܣ�%s", Reason(ret));
		return Call::Error(ResultCode::refuse, Reason(ret));
	}
	//ɾ����ǰ�ڵ������п����ҵ��ı��Լ���ص�ı����ݣ�ʣ���������������������ʱɾ��
	bigint startId, endId;
	for ( i = 0; i < lines.size(); i++ ) DelLine(lines[i], startId, endId);

	return Call::Success();
}

CallResult GridStore::MoveStart(mdk::Array<Grid::Point*> &points, 
	std::vector<bigint> &lineIds, std::vector<bigint> &pointIds,
	std::vector<bigint> &startPointIds, bool out, 
	bool moveAllLines, std::vector<Grid::FILTER> &selectLines, 
	std::vector<Grid::FILTER> &pointFilter, 
	Grid::Action act)
{
	bigint startId;
	bigint endId;
	int i = 0;
	int j = 0;
	Grid::Point *pPoint = NULL;
	mdk::uint64 start;
	for ( i = 0; i < startPointIds.size(); i++ )
	{
		pPoint = m_storePoint.FindPoint(startPointIds[i]);
		if ( NULL == pPoint ) continue;

		//ȡ��Ҫ�����ı�
		Grid::OwnLines *pLines = &pPoint->out;
		if ( !out ) pLines = &pPoint->in;
		Grid::OwnLines &lines = *pLines;
// 		m_pWorker->Logger().Info("run", "����ӵ��%d����", lines.m_count);
		//��ʼ������
		for ( j = 0; j < lines.m_count; j++ )
		{
			if ( NULL == lines.m_list[j].pointer ) 
			{
				if (!IsOwner(lines.m_list[j].id)) //���治�����Լ��ı�
				{
					lineIds.push_back(lines.m_list[j].id);
					continue;
				}
				if ( !LoadPoint(pPoint, out, j) ) continue;//���ض���ı����ݣ�����ɾ��
			}
 			if ( !moveAllLines && !IsSelected(lines.m_list[j].pointer->data, selectLines) ) continue;//���Ա����˱� 
			if ( Grid::unlink == act ) 
			{
				bigint startId, endId;
				DelLine(lines.m_list[j].id, startId, endId);
				continue;
			}

			//ȡ��Ҫ���Ķ���
			Grid::Point **pPoint = &(lines.m_list[j].pointer->pEndPoint);
			bigint pid = lines.m_list[j].pointer->endId;
			if (!out) 
			{
				pPoint = &(lines.m_list[j].pointer->pStartPoint);
				pid = lines.m_list[j].pointer->startId;
			}
			//��ʼ��鶥��
			if (NULL == (*pPoint)) 
			{
				if ( !IsOwner(pid) )//���治�����Լ��Ķ���
				{
					pointIds.push_back(pid);
					continue;
				}
				if ( !LoadLine(lines.m_list[j].pointer, !out) ) continue;//���رߵĶ������ݣ�������ɾ��
			}
			//�����ظ�����
			if ( m_isEndPoint )
			{
				if ( m_searchNo == (*pPoint)->searchNo ) 
				{
					if ( 30 > m_searchTime - (*pPoint)->searchTime ) continue;
				}
				(*pPoint)->searchNo = m_searchNo;
				(*pPoint)->searchTime = m_searchTime;
			}
 			if ( !IsSelected((*pPoint)->data, pointFilter) ) continue;//���Ա����˶���
			//ִ��act
			points.insert((*pPoint));
		}
		/*
			������ѭ����ִ��ɾ��
			��Ϊ�п�������а������������Ķ��㣬���磺
			�����ж��㣺1,2,3,4,5,6,7,8,9
			1������3������9
			ɾ��1,3���ӵĶ��㣬�����1,3�������Ľ����ɾ��3��9

			ѭ������
			��һ��ѭ����1�����ҵ���3�����ɾ����3
			�ڶ���ѭ����3���ң�3�����ݣ��˳�ѭ��
			9û��ɾ��
		*/
		if ( Grid::del == act )
		{
			int i = 0;
			for ( i = 0; i < points.size; i++ )
			{
				DelPoint(points.datas[i]->id);
			}
			points.clear();
		}
	}

	return Call::Success();
}

CallResult GridStore::MoveLine(mdk::Array<Grid::Point*> &points, std::vector<bigint> &pointIds, 
	std::vector<bigint> &lineIds, bool out, 
	bool moveAllLines, std::vector<Grid::FILTER> &selectLines, std::vector<Grid::FILTER> &pointFilter, 
	Grid::Action act)
{
	if ( !moveAllLines && 0 == lineIds.size() ) return Call::Error(ResultCode::paramError, "û�б�ѡ·��");

	int i = 0;
	Grid::Line *pLine = NULL;
	for ( i = 0; i < lineIds.size(); i++ )
	{
		pLine = m_storeLine.FindLine(lineIds[i]);
		if ( NULL == pLine ) continue;
		if ( !moveAllLines && !IsSelected(pLine->data, selectLines) ) continue;//���Ա����˱�
		if ( Grid::unlink == act ) 
		{
			bigint startId, endId;
			DelLine(pLine->id, startId, endId);
			continue;
		}

		Grid::Point **pPoint = &(pLine->pEndPoint);
		bigint pid = pLine->endId;
		if (!out) 
		{
			pPoint = &(pLine->pStartPoint);
			pid = pLine->startId;
		}
		if (NULL == (*pPoint)) 
		{
			if ( !IsOwner(pid) )//���治�����Լ����յ�
			{
				pointIds.push_back(pid);
				continue;
			}
			if ( !LoadLine(pLine, !out) ) continue;//�յ���ɾ��
		}
		//�����ظ�����
		if ( m_isEndPoint )
		{
			if ( m_searchNo == (*pPoint)->searchNo ) 
			{
				if ( 30 > m_searchTime - (*pPoint)->searchTime ) continue;
			}
			(*pPoint)->searchNo = m_searchNo;
			(*pPoint)->searchTime = m_searchTime;
		}
		if ( !IsSelected((*pPoint)->data, pointFilter) ) continue;//���Ա����˶���

		//ִ��act
		if ( Grid::del == act ) DelPoint(pid);
		else points.insert((*pPoint));
	}

	return Call::Success();
}

CallResult GridStore::FilterPoint(mdk::Array<Grid::Point*> &points, 
	std::vector<bigint> &ids, std::vector<Grid::FILTER> &pointFilter, Grid::Action act)
{
	int i = 0;
	Grid::Point *pPoint = NULL;
	mdk::uint64 start;
	for ( i = 0; i < ids.size(); i++ )
	{
		pPoint = m_storePoint.FindPoint(ids[i]);
		if ( NULL == pPoint ) continue;
		//�����ظ�����
		if ( m_isEndPoint )
		{
			if ( m_searchNo == pPoint->searchNo )
			{
				if ( 30 > m_searchTime - pPoint->searchTime ) continue;
			}
			pPoint->searchNo = m_searchNo;
			pPoint->searchTime = m_searchTime;
		}

		if ( !IsSelected(pPoint->data, pointFilter) ) continue;//���Ա����˶���
		//ִ��act
		if ( Grid::del == act ) DelPoint(pPoint->id);
		else points.insert(pPoint);
	}

	return Call::Success();
}

bool GridStore::IsSelected(mdk::Map &data, std::vector<Grid::FILTER> &filters)
{
	if ( 0 == filters.size() ) return true;//������

	int i = 0;
	mdk::Map::iterator it;
	Grid::FIELD *pField;
	for ( i = 0; i < filters.size(); i++ )
	{
 		Grid::FILTER &filter = filters[i];
		it = data.find((char*)filter.fieldName.c_str(), filter.fieldName.size(), filter.hashValue);
		if ( it.isEnd() ) 
		{
			if ( Grid::left == filter.cmpMode 
				|| Grid::leftEquals == filter.cmpMode ) continue;//�ֶβ�������Ϊֵ��С
			return false;
		}
		pField = (Grid::FIELD*)it.data();
		//�ַ����ֶαȽ�
		int ret = 0;
		if ( Grid::str == pField->type )
		{
			//str stream��������
			ret = 0;
			if ( pField->size < filter.field.size ) ret = -1;
			if ( pField->size > filter.field.size ) ret = 1;
			if ( 0 == ret ) 
			{
				int j = 0;
				for ( j = 0; j < pField->size; j++ )
				{
					if ( pField->data[j] == filter.field.data[j] ) continue;
					else if ( pField->data[j] < filter.field.data[j] ) 
					{
						ret = -1;
						break;
					}
					ret = 1;
					break;
				}
			}
		}
		else ret = pField->value - filter.field.value;//int�ֶαȽ�

		if ( 0 == ret )
		{
			if ( Grid::equals == filter.cmpMode ) continue;//=
			if ( Grid::leftEquals == filter.cmpMode ) continue;//<=
			if ( Grid::rightEquals == filter.cmpMode ) continue;//>=
		}
		if ( 0 > ret )
		{
			if ( Grid::unequals == filter.cmpMode ) continue;//!=
			if ( Grid::left == filter.cmpMode ) continue;//<
			if ( Grid::leftEquals == filter.cmpMode ) continue;//<=
		}
		if ( 0 < ret )
		{
			if ( Grid::unequals == filter.cmpMode ) continue;//!=
			if ( Grid::right == filter.cmpMode ) continue;//>
			if ( Grid::rightEquals == filter.cmpMode ) continue;//>=
		}

		return false;
	}

	return true;
}

CallResult GridStore::Move(bigint &searchNo, mdk::Array<Grid::Point*> &endPoints, std::vector<Grid::REMOTE_DATA> &remoteDatas, 
	bool isPointId, bool filterStartPoint, std::vector<bigint> &ids, 
	std::vector<Grid::SELECT_LINE> &selectLines, std::vector<Grid::FILTER> &pointFilter, Grid::Action act)
{
	if ( 0 == ids.size() ) return Call::Error(ResultCode::paramError, "ȱ���ƶ���ʼλ��");

	CallResult ret;
	m_isEndPoint = false;
	Grid::Action exeAct = Grid::get;//Ĭ��ֻȡ������Ϣ
	m_searchNo = searchNo;
	m_searchTime = time(NULL);
	if ( isPointId && filterStartPoint )//�������
	{
		if ( 0 == selectLines.size() ) //û��·�ߣ�ִ����ʵ����
		{
			exeAct = act;
			m_isEndPoint = true;
		}
		ret = FilterPoint(endPoints, ids, pointFilter, exeAct);
		if ( !ret.isSuccess ) return ret;
		if ( 0 == selectLines.size() ) return ret;//û��·�ߣ������յ�
		//��������б�
		ids.clear();
		int i = 0;
		for ( i = 0; i < endPoints.size; i++ ) ids.push_back(endPoints.datas[i]->id);
		endPoints.clear();
	}
	if ( 0 == selectLines.size() ) return Call::Error(ResultCode::paramError, "ȱ���ƶ�·��");//û��ѡ��·��

	int i = 0;
	int lastIndex = selectLines.size() - 1;
	exeAct = Grid::get;//�������1��·�ߣ�ֻȡ���㣬��Ϊ��ת���
	Grid::REMOTE_DATA remoteData;
	remoteData.lineIndex = 0;
	for ( ; true; remoteData.lineIndex++ )
	{
		if ( remoteData.lineIndex == lastIndex ) //���1��·���ƶ�����ִ����ʵ����
		{
			exeAct = act; 
			m_isEndPoint = true;
		}
		if ( Grid::unlink == act ) exeAct = act; 

		//��1��·��
		if ( isPointId ) 
		{
			ret = MoveStart(endPoints, remoteData.lineIds, remoteData.pointIds, 
				ids, selectLines[remoteData.lineIndex].moveOut, 
				selectLines[remoteData.lineIndex].moveAllLine, selectLines[remoteData.lineIndex].selectField, 
				pointFilter, exeAct);
		}
 		else 
		{
			ret = MoveLine(endPoints, remoteData.pointIds, 
				ids, selectLines[remoteData.lineIndex].moveOut, 
				selectLines[remoteData.lineIndex].moveAllLine, selectLines[remoteData.lineIndex].selectField, 
				pointFilter, exeAct);
		}
		if ( !ret.isSuccess ) return ret;
		if ( remoteData.lineIds.size() > 0 || remoteData.pointIds.size() > 0 ) 
		{
			remoteDatas.push_back(remoteData);
			remoteData.lineIds.clear();
			remoteData.pointIds.clear();
		}
		if ( remoteData.lineIndex == lastIndex ) break;//�������һ��·��

		ids.clear();
		for ( i = 0; i < endPoints.size; i++ ) ids.push_back(endPoints.datas[i]->id);
		endPoints.clear();
		isPointId = true;
	}

	return Call::Success();
}

bool GridStore::LoadLine(Grid::Line *pLine, bool loadStartPoint)
{
	if (loadStartPoint) //�������
	{
		pLine->pStartPoint = m_storePoint.FindPoint(pLine->startId);
		if ( NULL != pLine->pStartPoint ) return true;

		/*
			��������������Ϊ�п����Ǵ�������û��ɣ����²�ѯ�����á�
			������ƻ����������ݡ�
			����¼�������б���ʱ����
		*/
// 		//����ʧ�ܣ�������������
// 		Grid::Point *pEndPoint = pLine->pEndPoint;
// 		bigint startId;
// 		bigint endId;
// 		bigint lineId = pLine->id;
// 		m_storeLine.DelLine(lineId, startId, endId);//ɾ����
// 		//ɾ���յ����
// 		if ( !IsOwner(endId) ) return false;
// 		if ( NULL == pEndPoint )
// 		{
// 			pEndPoint = m_storePoint.FindPoint(endId);
// 			if ( NULL == pEndPoint ) return false;
// 		}
// 		pEndPoint->in.Del(lineId);
// 		m_storePoint.SavePoint(pEndPoint);
	}
	else //�����յ�
	{
		pLine->pEndPoint = m_storePoint.FindPoint(pLine->endId);
		if ( NULL != pLine->pEndPoint ) return true;

		/*
			��������������Ϊ�п����Ǵ�������û��ɣ����²�ѯ�����á�
			������ƻ����������ݡ�
			����¼�������б���ʱ����
		*/
// 		//����ʧ�ܣ�������������
// 		Grid::Point *pStartPoint = pLine->pStartPoint;
// 		bigint startId;
// 		bigint endId;
// 		bigint lineId = pLine->id;
// 		m_storeLine.DelLine(lineId, startId, endId);//ɾ����
// 		//ɾ��������
// 		if ( !IsOwner(startId) ) return false;
// 		if ( NULL == pStartPoint )
// 		{
// 			pStartPoint = m_storePoint.FindPoint(startId);
// 			if ( NULL == pStartPoint ) return false;
// 		}
// 		pStartPoint->out.Del(lineId);
// 		m_storePoint.SavePoint(pStartPoint);
	}

	return false;
}

bool GridStore::LoadPoint(Grid::Point *pPoint, bool out, int pos)
{
	if ( out )
	{
		pPoint->out.m_list[pos].pointer = m_storeLine.FindLine(pPoint->out.m_list[pos].id);
		if ( NULL != pPoint->out.m_list[pos].pointer ) return true;

		/*
			��������������Ϊ�п����Ǵ�������û��ɣ����²�ѯ�����á�
			������ƻ����������ݡ�
			����¼�������б���ʱ����
		*/
// 		//������������
// 		pPoint->out.DelByIndex(pos);
// 		m_storePoint.SavePoint(pPoint);
	}
	else
	{
		pPoint->in.m_list[pos].pointer = m_storeLine.FindLine(pPoint->in.m_list[pos].id);
		if ( NULL != pPoint->in.m_list[pos].pointer ) return true;

		/*
			��������������Ϊ�п����Ǵ�������û��ɣ����²�ѯ�����á�
			������ƻ����������ݡ�
			����¼�������б���ʱ����
		*/
// 		//������������
// 		pPoint->in.DelByIndex(pos);
// 		m_storePoint.SavePoint(pPoint);
	}

	return false;
}

CallResult GridStore::CreatePoints(std::vector<Grid::Point> &points)
{
	Grid::Result::Result ret = m_storePoint.CreatePoints(points);
	if ( Grid::Result::success != ret ) return Call::Error(ResultCode::refuse, Reason(ret));

	return Call::Success();
}
