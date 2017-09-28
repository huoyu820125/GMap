#include "DiskIO.h"
#include "mdk/include/mdk/mapi.h"
#include "GridStore.h"
#include "Worker.h"
#include "common/heap.h"
#include <cstring>

DiskIO::AutoCloseFile::AutoCloseFile( mdk::File *pFile )
{
	m_pFile = pFile;
}

DiskIO::AutoCloseFile::~AutoCloseFile()
{
	if ( NULL == m_pFile ) return;
	m_pFile->Close();
}

void DiskIO::AutoCloseFile::Detach()
{
	m_pFile = NULL;
}

DiskIO::DiskIO()
{
	m_pStore = NULL;
	m_streamSpace = 1024*1024*10;//10M�ռ��С
	m_stream = new char[m_streamSpace];//���ݶ���ת����д����̣��ʹӴ��̶�ȡ��ʱ��洢����(����)
	m_idxStreamSpace = 1024*1024*10;//10M���������ռ��С
	m_idxStream = new char[m_idxStreamSpace];//��������ת����д����̣��ʹӴ��̶�ȡ��ʱ��洢����(����)
}

DiskIO::~DiskIO()
{
}

void DiskIO::Init(GridStore *pStore)
{
	m_pStore = pStore;
	char exeDir[256];
	mdk::GetExeDir( exeDir, 256 );//ȡ�ÿ�ִ�г���λ��
	char dir[32];
	sprintf( dir, "node%d/data", m_pStore->NodeId());
	m_dataDir = exeDir;
	m_dataDir += "/";
	m_dataDir += dir;
}

std::string DiskIO::DataDir()
{
	return m_dataDir;
}

const char* DiskIO::CheckDataDir()
{
	if ( !mdk::File::Exist(m_dataDir) && !mdk::File::CreateDir(m_dataDir) ) return "�޷�����dataĿ¼";

	mdk::File permissions;
	mdk::File::Error ret = permissions.Open(m_dataDir, "permissions", mdk::File::write, mdk::File::assii);
	if ( mdk::File::fileNoExist == ret || mdk::File::dirNoExist == ret ) return "dataĿ¼Ȩ�޴���";
	permissions.Close();
	mdk::File::DelFile(m_dataDir, "permissions");

	bool success = true;
	m_pointIdx.SetFile(m_dataDir, "point.idx");	//���������ļ�
	if ( m_pointIdx.Exist() )
	{
		if ( !m_pointIdx.IsReadAble() ) return "dataĿ¼Ȩ�޴���";
		if ( !m_pointIdx.IsWriteAble() ) return "dataĿ¼Ȩ�޴���";
	}
	else 
	{
		if ( mdk::File::success != m_pointIdx.Open(mdk::File::write, mdk::File::assii) ) 
		{
			return "dataĿ¼Ȩ�޴���:���ܴ��������ļ�";
		}
		m_pointIdx.Close();
	}
	m_lineIdx.SetFile(m_dataDir, "line.idx");	//�������ļ�
	if ( m_lineIdx.Exist() )
	{
		if ( !m_lineIdx.IsReadAble() ) return "dataĿ¼Ȩ�޴���";
		if ( !m_lineIdx.IsWriteAble() ) return "dataĿ¼Ȩ�޴���";
	}
	else 
	{
		if ( mdk::File::success != m_lineIdx.Open(mdk::File::write, mdk::File::assii) )
		{
			return "dataĿ¼Ȩ�޴���:���ܴ��������ļ�";
		}
		m_lineIdx.Close();
	}
	
	char fileName[256];
	int i = 0;
	for ( i = 0; i < Grid::Limit::maxDBShardCount; i++ )
	{
		sprintf( fileName, "point%d.db", i + 1);
		m_pointData[i].SetFile(m_dataDir, fileName);	//���������ļ�
		if ( m_pointData[i].Exist() )
		{
			if ( !m_pointData[i].IsReadAble() ) return "dataĿ¼Ȩ�޴���";
			if ( !m_pointData[i].IsWriteAble() ) return "dataĿ¼Ȩ�޴���";
		}
		else 
		{
			if ( mdk::File::success != m_pointData[i].Open(mdk::File::write, mdk::File::assii) ) 
			{
				return "dataĿ¼Ȩ�޴���:���ܴ��������ļ�";
			}
			m_pointData[i].Close();
		}

		sprintf( fileName, "line%d.db", i + 1);
		m_lineData[i].SetFile(m_dataDir, fileName);	//�������ļ�
		if ( m_lineData[i].Exist() )
		{
			if ( !m_lineData[i].IsReadAble() ) return "dataĿ¼Ȩ�޴���";
			if ( !m_lineData[i].IsWriteAble() ) return "dataĿ¼Ȩ�޴���";
		}
		else 
		{
			if ( mdk::File::success != m_lineData[i].Open(mdk::File::write, mdk::File::assii) ) 
			{
				return "dataĿ¼Ȩ�޴���:���ܴ��������ļ�";
			}
			m_lineData[i].Close();
		}
	}

	return NULL;
}

const char* DiskIO::LoadPoints(std::map<bigint, Grid::Point*> &points)
{
	m_pointIdx.SetFile(m_dataDir, "point.idx");	//���������ļ�
	const char *reason = NULL;
	if ( m_pointIdx.Exist() )
	{
		if ( !m_pointIdx.IsReadAble() ) return "dataĿ¼Ȩ�޴���";
		if ( !m_pointIdx.IsWriteAble() ) return "dataĿ¼Ȩ�޴���";
		reason = ReadPointFile(m_pointIdx, points);
		if ( NULL != reason ) 
		{
			return reason;
		}
	}

	return NULL;
}

const char* DiskIO::ReadPointFile(mdk::File &idxFile, std::map<bigint, Grid::Point*> &points)
{
	idxFile.Close();
	DiskIO::AutoCloseFile autoClose(&idxFile);
	mdk::File::Error ret = idxFile.Open(mdk::File::read, mdk::File::assii);
	if ( mdk::File::fileNoExist == ret ) return "�����ļ���ʧ";
	if ( mdk::File::success != ret ) return "dataĿ¼Ȩ�޴���";

	bigint pointId;
	bigint idxPos = 0;
	bigint dataPos;
	Grid::Result::Result retReadPoint;
	Grid::Point newPoint;
	Grid::Point point;
	while (!idxFile.IsEnd())
	{
		if ( mdk::File::success != idxFile.Read(&pointId, sizeof(bigint)) ) return "�����ļ���";
		if ( mdk::File::success != idxFile.Read(&dataPos, sizeof(bigint)) ) return "�����ļ���";
		if ( 0 > dataPos ) return "�������ݴ���";
		if ( mdk::File::success != idxFile.Read(&point.diskSpace, sizeof(int)) ) return "�����ļ���";
		if ( 0 > point.diskSpace ) return "�������ݴ���";
		
		point.id = pointId;
		retReadPoint = ReadPoint(dataPos, point);
		if ( Grid::Result::success != retReadPoint )
		{
			if ( Grid::Result::noData == retReadPoint ) continue;
			return "�����ļ���";
		}

		Grid::Point *pPoint = Grid::Point::Alloc();
		pPoint->SetIdxPos(idxPos);
		pPoint->SetDataPos(dataPos);
		pPoint->id = point.id;
		pPoint->out = point.out;
		pPoint->in = point.in;
		pPoint->data = point.data;
		pPoint->searchNo = -1;//���1�μ�����=null
		pPoint->diskSpace = point.diskSpace;
		points[pPoint->id] = pPoint;
		idxPos += INDEX_SIZE;//�ƶ�����һ������������ʼλ��
		point = newPoint;//���³�ʼ�����Ͽ��뵱ǰ��������ָ��Ĺ���
	}

	return NULL;
}

const char* DiskIO::LoadLines(std::map<bigint, Grid::Line*> &lines)
{
	const char *reason;
	m_lineIdx.SetFile(m_dataDir, "line.idx");	//���������ļ�
	if ( m_lineIdx.Exist() )
	{
		if ( !m_lineIdx.IsReadAble() ) return "dataĿ¼Ȩ�޴���";
		if ( !m_lineIdx.IsWriteAble() ) return "dataĿ¼Ȩ�޴���";
		reason = ReadLineFile(m_lineIdx, lines);
		if ( NULL != reason ) 
		{
			return reason;
		}
	}

	return NULL;
}

const char* DiskIO::ReadLineFile(mdk::File &idxFile, std::map<bigint, Grid::Line*> &lines)
{
	idxFile.Close();
	DiskIO::AutoCloseFile autoClose(&idxFile);
	mdk::File::Error ret = idxFile.Open(mdk::File::read, mdk::File::assii);
	if ( mdk::File::fileNoExist == ret ) return "�����ļ���ʧ";
	if ( mdk::File::success != ret ) return "dataĿ¼Ȩ�޴���";

	bigint lineId;
	bigint idxPos = 0;
	bigint dataPos;
	Grid::Line newLine;
	Grid::Line line;
	Grid::Result::Result retReadLine;
	while (!idxFile.IsEnd())
	{
		if ( mdk::File::success != idxFile.Read(&line.id, sizeof(bigint)) ) return "�����ļ���";
		if ( 0 >= line.id ) return "�������ݴ���";
		if ( mdk::File::success != idxFile.Read(&dataPos, sizeof(bigint)) ) return "�����ļ���";
		if ( 0 > dataPos ) return "�������ݴ���";
		if ( mdk::File::success != idxFile.Read(&line.diskSpace, sizeof(int)) ) return "�����ļ���";
		if ( 0 >= line.diskSpace ) return "�������ݴ���";

		retReadLine = ReadLine(dataPos, line);
		if ( Grid::Result::success != retReadLine )
		{
			if ( Grid::Result::noData == retReadLine ) continue;
			return "�����ļ���";
		}
		Grid::Line *pLine = Grid::Line::Alloc();
		pLine->SetIdxPos(idxPos);
		pLine->SetDataPos(dataPos);
		pLine->id = line.id;
		pLine->startId = line.startId;
		pLine->endId = line.endId;
		pLine->pStartPoint = NULL;
		pLine->pEndPoint = NULL;
		pLine->data = line.data;
		pLine->diskSpace = line.diskSpace;
		lines[line.id] = pLine;
		idxPos += INDEX_SIZE;//�ƶ�����һ������������ʼλ��
		line = newLine;//���³�ʼ��, ���ͷ��ڴ棬�ڴ��ѱ����渴����
	}

	return NULL;
}

Grid::Result::Result DiskIO::SaveIndex(mdk::File &idxFile, bool isUpdated, bigint &idxPos, bigint id, const bigint &dataPos, int size)
{
	if ( 0 >= id ) return Grid::Result::paramError;
	if ( 0 >= size ) return Grid::Result::paramError;
	if ( 0 > dataPos ) return Grid::Result::idxError;
	idxFile.Close();
	bigint val64;
	char val8;
	if ( isUpdated ) 
	{
		if ( 0 > idxPos ) return Grid::Result::idxError; 
		if ( mdk::File::success 
			!= idxFile.Open(mdk::File::readWrite, mdk::File::assii) ) 
		{
			return Grid::Result::noFile;
		}
		if ( mdk::File::success != idxFile.Move(idxPos) ) return Grid::Result::idxError;
	}
	else
	{
		if ( mdk::File::success 
			!= idxFile.Open(mdk::File::add, mdk::File::assii) ) 
		{
			return Grid::Result::noPermissions;
		}
		idxPos = idxFile.Size();
	}
	if ( mdk::File::success != idxFile.Write(&id, sizeof(bigint)) ) return Grid::Result::noDiskSpace;
	if ( mdk::File::success != idxFile.Write((void*)&dataPos, sizeof(bigint)) ) return Grid::Result::noDiskSpace;
	if ( mdk::File::success != idxFile.Write((void*)&size, sizeof(int)) ) return Grid::Result::noDiskSpace;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::SaveDataStart(mdk::File &dbFile, bool isUpdated, bool &dataMoved, bigint &dataPos, int dataSize)
{
	dbFile.Close();
	if ( isUpdated ) //���²���
	{
		if ( 0 > dataPos ) return Grid::Result::idxError;

		if ( mdk::File::success != dbFile.Open(mdk::File::readWrite, mdk::File::assii) ) return Grid::Result::noFile;
		if ( mdk::File::success != dbFile.Move(dataPos) ) return Grid::Result::idxError;

		if ( !dataMoved ) return Grid::Result::success;

		//�ռ䲻����������״̬����Ϊɾ��
		char state = DiskIO::del;
		if ( mdk::File::success != dbFile.Write(&state, sizeof(char)) ) return Grid::Result::noDiskSpace;
		dbFile.ToTail();//�ƶ����ļ�ĩβ
	}
	else
	{
		if ( mdk::File::success != dbFile.Open(mdk::File::add, mdk::File::assii) ) return Grid::Result::noPermissions;
	}
	dataPos = dbFile.Size();//��¼���ݿ�ʼλ��

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::ReadDataStart(mdk::File &dbFile, const bigint &dataPos, int &size)
{
	if ( 0 > dataPos ) return Grid::Result::idxError;

	dbFile.Close();
	if ( mdk::File::success != dbFile.Open(mdk::File::read, mdk::File::assii) ) return Grid::Result::noFile;
	if ( mdk::File::success != dbFile.Move(dataPos) ) return Grid::Result::idxError;

	char state = DiskIO::del;
	if ( mdk::File::success != dbFile.Read(&state, sizeof(char)) ) return Grid::Result::dataDamage;
	if ( DiskIO::del == state ) return Grid::Result::noData;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::SavePoint(Grid::Point &point)
{
	if ( point.id <= 0 ) return Grid::Result::paramError;

	//ȡ��ȡ���ݺ�������ʼ
	bool isUpdated = point.IsPositioned();//�Ǹ��²���
	bigint idxPos = isUpdated?point.IdxPos():0;
	bigint dataPos = isUpdated?point.DataPos():0;

	int size = 0;
	if ( !Point2Stream(m_stream, point, DiskIO::normal, size, m_streamSpace) ) return Grid::Result::paramError;
	//��λ�����ļ�
	bigint index = point.id % Grid::Limit::maxDBShardCount;
	mdk::File &dbFile = m_pointData[index];
	DiskIO::AutoCloseFile autoCloseDB(&dbFile);//�Զ��ر��ļ�
	//д������
	Grid::Result::Result ret = SaveData(dbFile, dataPos, m_stream, size, point.diskSpace, !isUpdated);
	if ( Grid::Result::success != ret ) return ret;
	//�޸�����
	if ( !isUpdated || size > point.diskSpace ) //�����ݻ�������λ���ƶ��ˣ���Ҫ�޸�����
	{
		ret = SaveIndex(m_pointIdx, isUpdated, idxPos, point.id, dataPos, size);
		m_pointIdx.Close();
		if ( Grid::Result::success != ret ) return ret;
		point.SetIdxPos(idxPos);
		point.SetDataPos(dataPos);
	}
	point.diskSpace = size;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::SaveData(mdk::File &dbFile, bigint &dataPos, char *data, int size, int oldSize, bool isCreate)
{
	//��������
	if ( isCreate )//�ƶ����ļ�ĩβ
	{
		if ( mdk::File::success != dbFile.Open(mdk::File::add, mdk::File::assii) ) 
		{
			return Grid::Result::noPermissions;
		}
		dataPos = dbFile.Size();//��¼���ݿ�ʼλ��
		if ( mdk::File::success != dbFile.Write(data, size) ) return Grid::Result::noDiskSpace;

		return Grid::Result::success;
	}

	//�޸�����
	if ( mdk::File::success != dbFile.Open(mdk::File::readWrite, mdk::File::assii) ) 
	{
		return Grid::Result::noPermissions;
	}
	if ( mdk::File::success != dbFile.Move(dataPos) ) return Grid::Result::dataDamage;
	if ( size > oldSize )//�����ݳ��ȳ��������ݣ���ɾ�������ݣ�ֱ����ӵ�ĩβ
	{
		/*
			��д���ݣ����޸�������ָ���µ�����λ��
			�������д����ɺ󣬶ϵ磬ɱ���̣�����ö���Ϊupdataʧ�ܣ�������ָ�������ݣ������ȷ
		*/
		dbFile.ToTail();//�ƶ����ļ�ĩβ
		dataPos = dbFile.Size();//��¼���ݿ�ʼλ��
	}

	if ( mdk::File::success != dbFile.Write(data, size) ) return Grid::Result::noDiskSpace;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::ReadPoint(const bigint &dataPos, Grid::Point &point)
{
	if ( 0 > dataPos ) return Grid::Result::idxError;
	if ( point.id <= 0 ) return Grid::Result::idxError;

	//��λ�����ļ�
	int index = point.id % Grid::Limit::maxDBShardCount;
	mdk::File &dbFile = m_pointData[index];
	DiskIO::AutoCloseFile autoClosedb(&dbFile);
	//��ȡ����
	if ( mdk::File::success != dbFile.Open(mdk::File::read, mdk::File::assii) ) return Grid::Result::noFile;
	if ( mdk::File::success != dbFile.Move(dataPos) ) return Grid::Result::idxError;
	if ( mdk::File::success != dbFile.Read(m_stream, point.diskSpace) ) return Grid::Result::dataDamage;
	char state;
	int size;
	if ( !Stream2Point(point, state, size, m_stream, point.diskSpace) ) return Grid::Result::dataDamage;
	if ( DiskIO::del == state ) return Grid::Result::noData;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::DelPoint(bigint pointId, bigint dataPos)
{
	if ( pointId <= 0 ) return Grid::Result::paramError;
	if ( dataPos < 0 ) return Grid::Result::idxError;

	//��ʼ��������
	int index = pointId % Grid::Limit::maxDBShardCount;
	char state = DiskIO::del;
	mdk::File &dbFile = m_pointData[index];
	DiskIO::AutoCloseFile autoCloseDB(&dbFile);//�Զ��ر��ļ�
	dbFile.Close();
	if ( mdk::File::success != dbFile.Open(mdk::File::readWrite, mdk::File::assii) ) return Grid::Result::success;
	if ( mdk::File::success != dbFile.Move(dataPos) ) return Grid::Result::dataDamage;
	if ( mdk::File::success != dbFile.Write(&state, sizeof(char)) ) return Grid::Result::noDiskSpace;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::SaveLine(Grid::Line &line)
{
	if ( line.id <= 0 ) return Grid::Result::paramError;

	//ȡ��ȡ���ݺ�������ʼ
	bool isUpdated = line.IsPositioned();//�Ǹ��²���
	bigint idxPos = isUpdated?line.IdxPos():0;
	bigint dataPos = isUpdated?line.DataPos():0;

	int size;
	if ( !Line2Stream(m_stream, line, DiskIO::normal, size, m_streamSpace) ) return Grid::Result::paramError;
	//��λ�����ļ�
	int index = line.id % Grid::Limit::maxDBShardCount;
	mdk::File &dbFile = m_lineData[index];
	DiskIO::AutoCloseFile autoCloseDB(&dbFile);//�Զ��ر��ļ�
	//д������
	Grid::Result::Result ret = SaveData(dbFile, dataPos, m_stream, size, line.diskSpace, !isUpdated);
	if ( Grid::Result::success != ret ) return ret;
	//�޸�����
	if ( !isUpdated || size > line.diskSpace ) //�����ݻ�������λ���ƶ��ˣ���Ҫ�޸�����
	{
		ret = SaveIndex(m_lineIdx, isUpdated, idxPos, line.id, dataPos, size);
		m_lineIdx.Close();
		if ( Grid::Result::success != ret ) return ret;
		line.SetIdxPos(idxPos);
		line.SetDataPos(dataPos);
	}
	line.diskSpace = size;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::ReadLine(const bigint &dataPos, Grid::Line &line)
{
	if ( line.id <= 0 ) return Grid::Result::idxError;

	//��λ�����ļ�
	int index = line.id % Grid::Limit::maxDBShardCount;
	mdk::File &dbFile = m_lineData[index];
	DiskIO::AutoCloseFile autoClosedb(&dbFile);
	//��ȡ����
	if ( mdk::File::success != dbFile.Open(mdk::File::read, mdk::File::assii) ) return Grid::Result::noFile;
	if ( mdk::File::success != dbFile.Move(dataPos) ) return Grid::Result::idxError;
	if ( mdk::File::success != dbFile.Read(m_stream, line.diskSpace) ) return Grid::Result::dataDamage;
	char state;
	int size;
	if ( !Stream2Line(line, state, size, m_stream, line.diskSpace) ) return Grid::Result::dataDamage;
	if ( DiskIO::del == state ) return Grid::Result::noData;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::DelLine(bigint lineId, bigint dataPos)
{
	if ( lineId <= 0 ) return Grid::Result::paramError;
	if ( dataPos < 0 ) return Grid::Result::idxError;

	//��ʼ��������
	int index = lineId % Grid::Limit::maxDBShardCount;
	char state = DiskIO::del;
	mdk::File &dbFile = m_lineData[index];
	DiskIO::AutoCloseFile autoCloseDB(&dbFile);//�Զ��ر��ļ�
	dbFile.Close();
	if ( mdk::File::success != dbFile.Open(mdk::File::readWrite, mdk::File::assii) ) return Grid::Result::success;
	if ( mdk::File::success != dbFile.Move(dataPos) ) return Grid::Result::dataDamage;
	if ( mdk::File::success != dbFile.Write(&state, sizeof(char)) ) return Grid::Result::noDiskSpace;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::ReadIndex(mdk::File &idxFile, bigint id, bigint &idxPos, bigint &dataPos)
{
	idxFile.Close();
	DiskIO::AutoCloseFile autoClose(&idxFile);
	mdk::File::Error ret = idxFile.Open(mdk::File::read, mdk::File::assii);
	if ( mdk::File::fileNoExist == ret ) return Grid::Result::noFile;
	if ( mdk::File::success != ret ) return Grid::Result::noPermissions;

	bigint readId;
	idxPos = 0;
	Grid::Point point;
	Grid::Result::Result retReadPoint;
	while (!idxFile.IsEnd())
	{
		if ( mdk::File::success != idxFile.Read(&readId, sizeof(bigint)) ) return Grid::Result::idxDamage;
		if ( mdk::File::success != idxFile.Read(&dataPos, sizeof(bigint)) ) return Grid::Result::idxDamage;
		if ( 0 > dataPos ) return Grid::Result::idxError;
		if ( readId == id ) return Grid::Result::success;
		idxPos += INDEX_SIZE;
	}

	return Grid::Result::noData;
}

Grid::Result::Result DiskIO::ReadPoint(Grid::Point &point)
{
	if ( 0 >= point.id ) return Grid::Result::paramError;
	bigint idxPos;
	bigint dataPos;
	Grid::Result::Result ret = ReadIndex(m_pointIdx, point.id, idxPos, dataPos);
	if ( Grid::Result::success != ret ) return ret;

	return ReadPoint(dataPos, point);
}

Grid::Result::Result DiskIO::ReadLine(Grid::Line &line)
{
	if ( 0 >= line.id ) return Grid::Result::paramError;
	bigint idxPos;
	bigint dataPos;
	Grid::Result::Result ret = ReadIndex(m_lineIdx, line.id, idxPos, dataPos);
	if ( Grid::Result::success != ret ) return ret;

	return ReadLine(dataPos, line);
}

//�����б�ѹ���ֽ���
bool DiskIO::Fields2Stream(char *stream, mdk::Map &fields, int &size, unsigned int space)
{
	size = 0;
	//��������
	short sVal = fields.size();
// 	if ( sVal < 0 || sVal > Grid::Limit::maxFieldCount ) false;
	if ( size + sizeof(short) > space ) return false;
	memcpy(&stream[size], &sVal, sizeof(short) );
	size += sizeof(short);
	//�����б�
	mdk::Map::iterator it = fields.begin();
	Grid::FIELD *pField = NULL;
	char bVal;
	for ( ; !it.isEnd(); it++ )
	{
		pField = (Grid::FIELD*)it.data();
		//����������/0
		if ( size + sizeof(short) > space ) return false;
		sVal = it.key()->size;
// 		if ( 0 >= sVal || sVal > Grid::Limit::maxFieldNameSize ) return false;
		memcpy(&stream[size], &sVal, sizeof(short));
		size += sizeof(short);
		if ( size + sVal > space ) return false;
		memcpy(&stream[size], it.key()->data, it.key()->size);
		size += it.key()->size;
		//��������
		if ( size + sizeof(char) > space ) return false;
		bVal = pField->type;
		memcpy(&stream[size], &bVal, sizeof(char));
		size += sizeof(char);
		//����ֵ
		if ( Grid::str == pField->type )
		{
			if ( size + sizeof(int) > space ) return false;
			memcpy(&stream[size], &pField->size, sizeof(int));
// 			if ( 0 >= pField->size || Grid::Limit::maxFieldSize < pField->size ) return false;
			size += sizeof(int);
			if ( size + pField->size > space ) return false;
			memcpy(&stream[size], pField->data, pField->size);
			size += pField->size;
		}
		else 
		{
			if ( size + sizeof(bigint) > space ) return false;
			memcpy(&stream[size], &pField->value, sizeof(bigint));
			size += sizeof(bigint);
		}
	}

	return true;
}

bool DiskIO::Stream2Fields(mdk::Map &fields, int &size, char *stream, unsigned int space)
{
	size = 0;
	//��������
	if ( size + sizeof(short) > space ) return false;
	short count = 0;
	memcpy(&count, &stream[size], sizeof(short) );
	if ( count < 0 || count > Grid::Limit::maxFieldCount ) false;
	size += sizeof(short);

	//�����б�
	Grid::FIELD *pField = NULL;
	char bVal;
	short sVal;
	int i = 0;
	char fieldName[Grid::Limit::maxFieldNameSize + 1];
	for ( ; i < count; i++ )
	{
		pField = new Grid::FIELD;
		//����������/0
		if ( size + sizeof(short) > space ) return false;
		memcpy(&sVal, &stream[size], sizeof(short));
		if ( 0 >= sVal || sVal > Grid::Limit::maxFieldNameSize ) return false;
		size += sizeof(short);

		if ( size + sVal > space ) return false;
		memcpy(fieldName, &stream[size], sVal);
		size += sVal;
		fieldName[sVal] = 0;

		//��������
		if ( size + sizeof(char) > space ) return false;
		memcpy(&bVal, &stream[size], sizeof(char));
		pField->type = (Grid::DataType)bVal;
		size += sizeof(char);

		//����ֵ
		if ( Grid::str == pField->type )
		{
			if ( size + sizeof(int) > space ) return false;
			memcpy(&pField->size, &stream[size], sizeof(int));
			if ( 0 >= pField->size || Grid::Limit::maxFieldSize < pField->size ) return false;
			size += sizeof(int);
			if ( size + pField->size > space ) return false;
			memcpy(pField->data, &stream[size], pField->size);
			size += pField->size;
		}
		else 
		{
			if ( size + sizeof(bigint) > space ) return false;
			memcpy(&pField->value, &stream[size], sizeof(bigint));
			size += sizeof(bigint);
		}
		fields.insert(fieldName, sVal, pField);
	}

	return true;
}

//��������ѹ���ֽ���
bool DiskIO::Point2Stream(char *stream, Grid::Point &point, char state, int &size, unsigned int space)
{
	size = 0;
	//����״̬
	if ( size + sizeof(char) > space ) return false;
// 	if ( DiskIO::normal != state && DiskIO::del != state ) return false;
	memcpy(&stream[size], &state, sizeof(char) );
	size += sizeof(char);
	//����id
// 	if ( 0 >= point.id ) return false;
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&stream[size], &point.id, sizeof(bigint) );
	size += sizeof(bigint);
	//������
// 	if ( 0 > point.out.m_count ) return false;
	if ( size + sizeof(int) > space ) return false;
	int iVal = 0;
	memcpy(&stream[size], &point.out.m_count, sizeof(int) );
	size += sizeof(int);
	//id�б�
	int i = 0;
	for (i = 0; i < point.out.m_count; i++ )
	{
// 		if ( 0 >= point.out.m_list[i].id ) return false;
		if ( size + sizeof(bigint) > space ) return false;
		memcpy(&stream[size], &point.out.m_list[i].id, sizeof(bigint) );
		size += sizeof(bigint);
	}
	//�����
// 	if ( 0 > point.in.m_count ) return false;
	if ( size + sizeof(int) > space ) return false;
	memcpy(&stream[size], &point.in.m_count, sizeof(int) );
	size += sizeof(int);
	//id�б�
	for (i = 0; i < point.in.m_count; i++ )
	{
// 		if ( 0 >= point.in.m_list[i].id ) return false;
		if ( size + sizeof(bigint) > space ) return false;
		memcpy(&stream[size], &point.in.m_list[i].id, sizeof(bigint) );
		size += sizeof(bigint);
	}

	//�����б�
	int fieldsSize;
	if ( !Fields2Stream(&stream[size], point.data, fieldsSize, space - size) ) return false;
	size += fieldsSize;

	return true;
}

bool DiskIO::Stream2Point(Grid::Point &point, char &state, int &size, char *stream, unsigned int space)
{
	size = 0;
	//����״̬
	if ( size + sizeof(char) > space ) return false;
	memcpy(&state, &stream[size], sizeof(char) );
	size += sizeof(char);
	//����id
	if ( size + sizeof(bigint) > space ) return false;
	bigint id;
	memcpy(&id, &stream[size], sizeof(bigint) );
	if ( id != point.id ) return false;
	size += sizeof(bigint);
	//������
	if ( size + sizeof(int) > space ) return false;
	int iVal = 0;
	memcpy(&iVal, &stream[size], sizeof(int) );
	if ( 0 > iVal ) return false;
	size += sizeof(int);
	//id�б�
	int i = 0;
	for (i = 0; i < iVal; i++ )
	{
		if ( size + sizeof(bigint) > space ) return false;
		memcpy(&id, &stream[size], sizeof(bigint) );
		if ( 0 >= id ) return false;
		point.out.Add(id);
		size += sizeof(bigint);
	}
	//�����
	if ( size + sizeof(int) > space ) return false;
	memcpy(&iVal, &stream[size], sizeof(int) );
	if ( 0 > iVal ) return false;
	size += sizeof(int);
	//id�б�
	for (i = 0; i < iVal; i++ )
	{
		if ( size + sizeof(bigint) > space ) return false;
		memcpy(&id, &stream[size], sizeof(bigint) );
		if ( 0 >= id ) return false;
		point.out.Add(id);
		size += sizeof(bigint);
	}

	//�����б�
	int fieldsSize;
	if ( !Stream2Fields(point.data, fieldsSize, &stream[size], space - size) ) return false;
	size += fieldsSize;

	return true;
}

//������ѹ���ֽ���
bool DiskIO::Line2Stream(char *stream, Grid::Line &line, char state, int &size, unsigned int space)
{
	size = 0;
	//����״̬
	if ( size + sizeof(char) > space ) return false;
// 	if ( DiskIO::normal != state && DiskIO::del != state ) return false;
	memcpy(&stream[size], &state, sizeof(char) );
	size += sizeof(char);
	//����id
// 	if ( 0 >= line.id ) return false;
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&stream[size], &line.id, sizeof(bigint) );
	size += sizeof(bigint);
	//���id
// 	if ( 0 >= line.startId ) return false;
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&stream[size], &line.startId, sizeof(bigint) );
	size += sizeof(bigint);
	//�յ�id
// 	if ( 0 >= line.endId ) return false;
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&stream[size], &line.endId, sizeof(bigint) );
	size += sizeof(bigint);

	//�����б�
	int fieldsSize;
	if ( !Fields2Stream(&stream[size], line.data, fieldsSize, space - size) ) return false;
	size += fieldsSize;

	return true;
}

bool DiskIO::Stream2Line(Grid::Line &line, char &state, int &size, char *stream, unsigned int space)
{
	size = 0;
	//����״̬
	if ( size + sizeof(char) > space ) return false;
	memcpy(&state, &stream[size], sizeof(char) );
	size += sizeof(char);
	//����id
	if ( size + sizeof(bigint) > space ) return false;
	bigint id;
	memcpy(&id, &stream[size], sizeof(bigint) );
	if ( id != line.id ) return false;
	size += sizeof(bigint);
	//���id
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&line.startId, &stream[size], sizeof(bigint) );
	if ( 0 >= line.startId ) return false;
	size += sizeof(bigint);
	//�յ�id
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&line.endId, &stream[size], sizeof(bigint) );
	if ( 0 >= line.endId ) return false;
	size += sizeof(bigint);

	//�����б�
	int fieldsSize;
	if ( !Stream2Fields(line.data, fieldsSize, &stream[size], space - size) ) return false;
	size += fieldsSize;

	return true;
}

Grid::Result::Result DiskIO::CreatePoints(std::vector<Grid::Point*> &points)
{
	//�������,�������ļ�����
	int count = points.size();
	if ( 0 == count ) return Grid::Result::paramError;
	int i = 0;
	std::map<bigint, std::vector<Grid::Point*> > pointsMap;
	bigint index = 0;
	for ( ; i < count; i++ )
	{
		if ( 0 >= points[i]->id ) return Grid::Result::paramError;
		index = points[i]->id % Grid::Limit::maxDBShardCount;
		pointsMap[index].push_back(points[i]);
	}

	mdk::File &idxFile = m_pointIdx;
	DiskIO::AutoCloseFile autoCloseIdx(&idxFile);//�Զ��ر��ļ�
	if ( mdk::File::success != idxFile.Open(mdk::File::add, mdk::File::assii) ) return Grid::Result::noPermissions;
	bigint idxPos = idxFile.Size();//��¼������ʼλ��
	bigint dataPos = 0;//���ݿ�ʼλ��
	int size = 0;//���ݳ���
	int idxSize = 0;//�������ݳ���
	//������Ҫ���µ��ļ�
	std::map<bigint, std::vector<Grid::Point*> >::iterator it;
	for ( it = pointsMap.begin(); it != pointsMap.end(); it++ )
	{
		//��λ�����ļ�
		mdk::File &dbFile = m_pointData[it->first];
		DiskIO::AutoCloseFile autoCloseDB(&dbFile);//�Զ��ر��ļ�
		if ( mdk::File::success != dbFile.Open(mdk::File::add, mdk::File::assii) ) 
		{
			return Grid::Result::noPermissions;
		}
		dataPos = dbFile.Size();//��¼���ݿ�ʼλ��
		//׼����һ���ļ�д��
		std::vector<Grid::Point*> &points = it->second;
		count = points.size();
		idxSize = size = 0;
		for ( i = 0; i < count; i++ )
		{
			if ( idxSize + INDEX_SIZE > m_idxStreamSpace ) 
			{//�������Ѿ�װ������д�����
				//д������
				if ( mdk::File::success != dbFile.Write(m_stream, size) ) return Grid::Result::noDiskSpace;
				//д������
				if ( mdk::File::success != idxFile.Write(&m_idxStream, idxSize) ) return Grid::Result::noDiskSpace;
				//�������ʣ�����
				idxSize = size = 0;
				continue;
			}

			//���������
			if ( !Point2Stream(&m_stream[size], *(points[i]), 
				DiskIO::normal, points[i]->diskSpace, m_streamSpace) ) 
			{//�����Ѿ�װ������д�����
				//д������
				if ( mdk::File::success != dbFile.Write(m_stream, size) ) return Grid::Result::noDiskSpace;
				//д������
				if ( mdk::File::success != idxFile.Write(&m_idxStream, idxSize) ) return Grid::Result::noDiskSpace;
				//�������ʣ�����
				idxSize = size = 0;
				continue;
			}
			size += points[i]->diskSpace;//��������������
			//���������
			memcpy(&m_idxStream[idxSize], &(points[i]->id), sizeof(bigint));//����id
			idxSize += sizeof(bigint);//��������������
			memcpy(&m_idxStream[idxSize], &dataPos, sizeof(bigint));//���ݿ�ʼλ��
			idxSize += sizeof(bigint);//��������������
			memcpy(&m_idxStream[idxSize], &(points[i]->diskSpace), sizeof(int));//���ݳ���
			idxSize += sizeof(int);//��������������

			//�������������ݿ�ʼλ��
			points[i]->SetIdxPos(idxPos);
			points[i]->SetDataPos(dataPos);
			//������һ�������������ļ��������ļ��п�ʼλ��
			idxPos += INDEX_SIZE;
			dataPos += points[i]->diskSpace;
		}
		//д������
		if ( mdk::File::success != dbFile.Write(m_stream, size) ) return Grid::Result::noDiskSpace;
		//д������
		if ( mdk::File::success != idxFile.Write(m_idxStream, idxSize) ) return Grid::Result::noDiskSpace;
	}

	return Grid::Result::success;
}
