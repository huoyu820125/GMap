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
	m_streamSpace = 1024*1024*10;//10M空间大小
	m_stream = new char[m_streamSpace];//数据对象转成流写入磁盘，和从磁盘读取流时候存储数据(复用)
	m_idxStreamSpace = 1024*1024*10;//10M数据索引空间大小
	m_idxStream = new char[m_idxStreamSpace];//数据索引转成流写入磁盘，和从磁盘读取流时候存储数据(复用)
}

DiskIO::~DiskIO()
{
}

void DiskIO::Init(GridStore *pStore)
{
	m_pStore = pStore;
	char exeDir[256];
	mdk::GetExeDir( exeDir, 256 );//取得可执行程序位置
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
	if ( !mdk::File::Exist(m_dataDir) && !mdk::File::CreateDir(m_dataDir) ) return "无法创建data目录";

	mdk::File permissions;
	mdk::File::Error ret = permissions.Open(m_dataDir, "permissions", mdk::File::write, mdk::File::assii);
	if ( mdk::File::fileNoExist == ret || mdk::File::dirNoExist == ret ) return "data目录权限错误";
	permissions.Close();
	mdk::File::DelFile(m_dataDir, "permissions");

	bool success = true;
	m_pointIdx.SetFile(m_dataDir, "point.idx");	//顶点索引文件
	if ( m_pointIdx.Exist() )
	{
		if ( !m_pointIdx.IsReadAble() ) return "data目录权限错误";
		if ( !m_pointIdx.IsWriteAble() ) return "data目录权限错误";
	}
	else 
	{
		if ( mdk::File::success != m_pointIdx.Open(mdk::File::write, mdk::File::assii) ) 
		{
			return "data目录权限错误:不能创建数据文件";
		}
		m_pointIdx.Close();
	}
	m_lineIdx.SetFile(m_dataDir, "line.idx");	//边索引文件
	if ( m_lineIdx.Exist() )
	{
		if ( !m_lineIdx.IsReadAble() ) return "data目录权限错误";
		if ( !m_lineIdx.IsWriteAble() ) return "data目录权限错误";
	}
	else 
	{
		if ( mdk::File::success != m_lineIdx.Open(mdk::File::write, mdk::File::assii) )
		{
			return "data目录权限错误:不能创建数据文件";
		}
		m_lineIdx.Close();
	}
	
	char fileName[256];
	int i = 0;
	for ( i = 0; i < Grid::Limit::maxDBShardCount; i++ )
	{
		sprintf( fileName, "point%d.db", i + 1);
		m_pointData[i].SetFile(m_dataDir, fileName);	//顶点数据文件
		if ( m_pointData[i].Exist() )
		{
			if ( !m_pointData[i].IsReadAble() ) return "data目录权限错误";
			if ( !m_pointData[i].IsWriteAble() ) return "data目录权限错误";
		}
		else 
		{
			if ( mdk::File::success != m_pointData[i].Open(mdk::File::write, mdk::File::assii) ) 
			{
				return "data目录权限错误:不能创建数据文件";
			}
			m_pointData[i].Close();
		}

		sprintf( fileName, "line%d.db", i + 1);
		m_lineData[i].SetFile(m_dataDir, fileName);	//边数据文件
		if ( m_lineData[i].Exist() )
		{
			if ( !m_lineData[i].IsReadAble() ) return "data目录权限错误";
			if ( !m_lineData[i].IsWriteAble() ) return "data目录权限错误";
		}
		else 
		{
			if ( mdk::File::success != m_lineData[i].Open(mdk::File::write, mdk::File::assii) ) 
			{
				return "data目录权限错误:不能创建数据文件";
			}
			m_lineData[i].Close();
		}
	}

	return NULL;
}

const char* DiskIO::LoadPoints(std::map<bigint, Grid::Point*> &points)
{
	m_pointIdx.SetFile(m_dataDir, "point.idx");	//顶点索引文件
	const char *reason = NULL;
	if ( m_pointIdx.Exist() )
	{
		if ( !m_pointIdx.IsReadAble() ) return "data目录权限错误";
		if ( !m_pointIdx.IsWriteAble() ) return "data目录权限错误";
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
	if ( mdk::File::fileNoExist == ret ) return "索引文件丢失";
	if ( mdk::File::success != ret ) return "data目录权限错误";

	bigint pointId;
	bigint idxPos = 0;
	bigint dataPos;
	Grid::Result::Result retReadPoint;
	Grid::Point newPoint;
	Grid::Point point;
	while (!idxFile.IsEnd())
	{
		if ( mdk::File::success != idxFile.Read(&pointId, sizeof(bigint)) ) return "索引文件损坏";
		if ( mdk::File::success != idxFile.Read(&dataPos, sizeof(bigint)) ) return "索引文件损坏";
		if ( 0 > dataPos ) return "索引数据错误";
		if ( mdk::File::success != idxFile.Read(&point.diskSpace, sizeof(int)) ) return "索引文件损坏";
		if ( 0 > point.diskSpace ) return "索引数据错误";
		
		point.id = pointId;
		retReadPoint = ReadPoint(dataPos, point);
		if ( Grid::Result::success != retReadPoint )
		{
			if ( Grid::Result::noData == retReadPoint ) continue;
			return "数据文件损坏";
		}

		Grid::Point *pPoint = Grid::Point::Alloc();
		pPoint->SetIdxPos(idxPos);
		pPoint->SetDataPos(dataPos);
		pPoint->id = point.id;
		pPoint->out = point.out;
		pPoint->in = point.in;
		pPoint->data = point.data;
		pPoint->searchNo = -1;//最后1次检索号=null
		pPoint->diskSpace = point.diskSpace;
		points[pPoint->id] = pPoint;
		idxPos += INDEX_SIZE;//移动到下一条数据索引起始位置
		point = newPoint;//重新初始化，断开与当前顶点数据指针的关联
	}

	return NULL;
}

const char* DiskIO::LoadLines(std::map<bigint, Grid::Line*> &lines)
{
	const char *reason;
	m_lineIdx.SetFile(m_dataDir, "line.idx");	//顶点索引文件
	if ( m_lineIdx.Exist() )
	{
		if ( !m_lineIdx.IsReadAble() ) return "data目录权限错误";
		if ( !m_lineIdx.IsWriteAble() ) return "data目录权限错误";
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
	if ( mdk::File::fileNoExist == ret ) return "索引文件丢失";
	if ( mdk::File::success != ret ) return "data目录权限错误";

	bigint lineId;
	bigint idxPos = 0;
	bigint dataPos;
	Grid::Line newLine;
	Grid::Line line;
	Grid::Result::Result retReadLine;
	while (!idxFile.IsEnd())
	{
		if ( mdk::File::success != idxFile.Read(&line.id, sizeof(bigint)) ) return "索引文件损坏";
		if ( 0 >= line.id ) return "索引数据错误";
		if ( mdk::File::success != idxFile.Read(&dataPos, sizeof(bigint)) ) return "索引文件损坏";
		if ( 0 > dataPos ) return "索引数据错误";
		if ( mdk::File::success != idxFile.Read(&line.diskSpace, sizeof(int)) ) return "索引文件损坏";
		if ( 0 >= line.diskSpace ) return "索引数据错误";

		retReadLine = ReadLine(dataPos, line);
		if ( Grid::Result::success != retReadLine )
		{
			if ( Grid::Result::noData == retReadLine ) continue;
			return "数据文件损坏";
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
		idxPos += INDEX_SIZE;//移动到下一条数据索引起始位置
		line = newLine;//重新初始化, 不释放内存，内存已被上面复制走
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
	if ( isUpdated ) //更新操作
	{
		if ( 0 > dataPos ) return Grid::Result::idxError;

		if ( mdk::File::success != dbFile.Open(mdk::File::readWrite, mdk::File::assii) ) return Grid::Result::noFile;
		if ( mdk::File::success != dbFile.Move(dataPos) ) return Grid::Result::idxError;

		if ( !dataMoved ) return Grid::Result::success;

		//空间不够，将数据状态设置为删除
		char state = DiskIO::del;
		if ( mdk::File::success != dbFile.Write(&state, sizeof(char)) ) return Grid::Result::noDiskSpace;
		dbFile.ToTail();//移动到文件末尾
	}
	else
	{
		if ( mdk::File::success != dbFile.Open(mdk::File::add, mdk::File::assii) ) return Grid::Result::noPermissions;
	}
	dataPos = dbFile.Size();//记录数据开始位置

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

	//取得取数据和索引起始
	bool isUpdated = point.IsPositioned();//是更新操作
	bigint idxPos = isUpdated?point.IdxPos():0;
	bigint dataPos = isUpdated?point.DataPos():0;

	int size = 0;
	if ( !Point2Stream(m_stream, point, DiskIO::normal, size, m_streamSpace) ) return Grid::Result::paramError;
	//定位数据文件
	bigint index = point.id % Grid::Limit::maxDBShardCount;
	mdk::File &dbFile = m_pointData[index];
	DiskIO::AutoCloseFile autoCloseDB(&dbFile);//自动关闭文件
	//写入数据
	Grid::Result::Result ret = SaveData(dbFile, dataPos, m_stream, size, point.diskSpace, !isUpdated);
	if ( Grid::Result::success != ret ) return ret;
	//修改索引
	if ( !isUpdated || size > point.diskSpace ) //新数据或者数据位置移动了，需要修改索引
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
	//创建数据
	if ( isCreate )//移动到文件末尾
	{
		if ( mdk::File::success != dbFile.Open(mdk::File::add, mdk::File::assii) ) 
		{
			return Grid::Result::noPermissions;
		}
		dataPos = dbFile.Size();//记录数据开始位置
		if ( mdk::File::success != dbFile.Write(data, size) ) return Grid::Result::noDiskSpace;

		return Grid::Result::success;
	}

	//修改数据
	if ( mdk::File::success != dbFile.Open(mdk::File::readWrite, mdk::File::assii) ) 
	{
		return Grid::Result::noPermissions;
	}
	if ( mdk::File::success != dbFile.Move(dataPos) ) return Grid::Result::dataDamage;
	if ( size > oldSize )//新数据长度超过旧数据，不删除旧数据，直接添加到末尾
	{
		/*
			先写数据，后修改索引，指向新的数据位置
			如果数据写入完成后，断电，杀进程，则调用端认为updata失败，索引还指着老数据，结果正确
		*/
		dbFile.ToTail();//移动到文件末尾
		dataPos = dbFile.Size();//记录数据开始位置
	}

	if ( mdk::File::success != dbFile.Write(data, size) ) return Grid::Result::noDiskSpace;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::ReadPoint(const bigint &dataPos, Grid::Point &point)
{
	if ( 0 > dataPos ) return Grid::Result::idxError;
	if ( point.id <= 0 ) return Grid::Result::idxError;

	//定位数据文件
	int index = point.id % Grid::Limit::maxDBShardCount;
	mdk::File &dbFile = m_pointData[index];
	DiskIO::AutoCloseFile autoClosedb(&dbFile);
	//读取数据
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

	//开始保存数据
	int index = pointId % Grid::Limit::maxDBShardCount;
	char state = DiskIO::del;
	mdk::File &dbFile = m_pointData[index];
	DiskIO::AutoCloseFile autoCloseDB(&dbFile);//自动关闭文件
	dbFile.Close();
	if ( mdk::File::success != dbFile.Open(mdk::File::readWrite, mdk::File::assii) ) return Grid::Result::success;
	if ( mdk::File::success != dbFile.Move(dataPos) ) return Grid::Result::dataDamage;
	if ( mdk::File::success != dbFile.Write(&state, sizeof(char)) ) return Grid::Result::noDiskSpace;

	return Grid::Result::success;
}

Grid::Result::Result DiskIO::SaveLine(Grid::Line &line)
{
	if ( line.id <= 0 ) return Grid::Result::paramError;

	//取得取数据和索引起始
	bool isUpdated = line.IsPositioned();//是更新操作
	bigint idxPos = isUpdated?line.IdxPos():0;
	bigint dataPos = isUpdated?line.DataPos():0;

	int size;
	if ( !Line2Stream(m_stream, line, DiskIO::normal, size, m_streamSpace) ) return Grid::Result::paramError;
	//定位数据文件
	int index = line.id % Grid::Limit::maxDBShardCount;
	mdk::File &dbFile = m_lineData[index];
	DiskIO::AutoCloseFile autoCloseDB(&dbFile);//自动关闭文件
	//写入数据
	Grid::Result::Result ret = SaveData(dbFile, dataPos, m_stream, size, line.diskSpace, !isUpdated);
	if ( Grid::Result::success != ret ) return ret;
	//修改索引
	if ( !isUpdated || size > line.diskSpace ) //新数据或者数据位置移动了，需要修改索引
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

	//定位数据文件
	int index = line.id % Grid::Limit::maxDBShardCount;
	mdk::File &dbFile = m_lineData[index];
	DiskIO::AutoCloseFile autoClosedb(&dbFile);
	//读取数据
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

	//开始保存数据
	int index = lineId % Grid::Limit::maxDBShardCount;
	char state = DiskIO::del;
	mdk::File &dbFile = m_lineData[index];
	DiskIO::AutoCloseFile autoCloseDB(&dbFile);//自动关闭文件
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

//属性列表压入字节流
bool DiskIO::Fields2Stream(char *stream, mdk::Map &fields, int &size, unsigned int space)
{
	size = 0;
	//属性数量
	short sVal = fields.size();
// 	if ( sVal < 0 || sVal > Grid::Limit::maxFieldCount ) false;
	if ( size + sizeof(short) > space ) return false;
	memcpy(&stream[size], &sVal, sizeof(short) );
	size += sizeof(short);
	//属性列表
	mdk::Map::iterator it = fields.begin();
	Grid::FIELD *pField = NULL;
	char bVal;
	for ( ; !it.isEnd(); it++ )
	{
		pField = (Grid::FIELD*)it.data();
		//属性名不带/0
		if ( size + sizeof(short) > space ) return false;
		sVal = it.key()->size;
// 		if ( 0 >= sVal || sVal > Grid::Limit::maxFieldNameSize ) return false;
		memcpy(&stream[size], &sVal, sizeof(short));
		size += sizeof(short);
		if ( size + sVal > space ) return false;
		memcpy(&stream[size], it.key()->data, it.key()->size);
		size += it.key()->size;
		//属性类型
		if ( size + sizeof(char) > space ) return false;
		bVal = pField->type;
		memcpy(&stream[size], &bVal, sizeof(char));
		size += sizeof(char);
		//属性值
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
	//属性数量
	if ( size + sizeof(short) > space ) return false;
	short count = 0;
	memcpy(&count, &stream[size], sizeof(short) );
	if ( count < 0 || count > Grid::Limit::maxFieldCount ) false;
	size += sizeof(short);

	//属性列表
	Grid::FIELD *pField = NULL;
	char bVal;
	short sVal;
	int i = 0;
	char fieldName[Grid::Limit::maxFieldNameSize + 1];
	for ( ; i < count; i++ )
	{
		pField = new Grid::FIELD;
		//属性名不带/0
		if ( size + sizeof(short) > space ) return false;
		memcpy(&sVal, &stream[size], sizeof(short));
		if ( 0 >= sVal || sVal > Grid::Limit::maxFieldNameSize ) return false;
		size += sizeof(short);

		if ( size + sVal > space ) return false;
		memcpy(fieldName, &stream[size], sVal);
		size += sVal;
		fieldName[sVal] = 0;

		//属性类型
		if ( size + sizeof(char) > space ) return false;
		memcpy(&bVal, &stream[size], sizeof(char));
		pField->type = (Grid::DataType)bVal;
		size += sizeof(char);

		//属性值
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

//顶点数据压入字节流
bool DiskIO::Point2Stream(char *stream, Grid::Point &point, char state, int &size, unsigned int space)
{
	size = 0;
	//数据状态
	if ( size + sizeof(char) > space ) return false;
// 	if ( DiskIO::normal != state && DiskIO::del != state ) return false;
	memcpy(&stream[size], &state, sizeof(char) );
	size += sizeof(char);
	//数据id
// 	if ( 0 >= point.id ) return false;
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&stream[size], &point.id, sizeof(bigint) );
	size += sizeof(bigint);
	//出边数
// 	if ( 0 > point.out.m_count ) return false;
	if ( size + sizeof(int) > space ) return false;
	int iVal = 0;
	memcpy(&stream[size], &point.out.m_count, sizeof(int) );
	size += sizeof(int);
	//id列表
	int i = 0;
	for (i = 0; i < point.out.m_count; i++ )
	{
// 		if ( 0 >= point.out.m_list[i].id ) return false;
		if ( size + sizeof(bigint) > space ) return false;
		memcpy(&stream[size], &point.out.m_list[i].id, sizeof(bigint) );
		size += sizeof(bigint);
	}
	//入边数
// 	if ( 0 > point.in.m_count ) return false;
	if ( size + sizeof(int) > space ) return false;
	memcpy(&stream[size], &point.in.m_count, sizeof(int) );
	size += sizeof(int);
	//id列表
	for (i = 0; i < point.in.m_count; i++ )
	{
// 		if ( 0 >= point.in.m_list[i].id ) return false;
		if ( size + sizeof(bigint) > space ) return false;
		memcpy(&stream[size], &point.in.m_list[i].id, sizeof(bigint) );
		size += sizeof(bigint);
	}

	//属性列表
	int fieldsSize;
	if ( !Fields2Stream(&stream[size], point.data, fieldsSize, space - size) ) return false;
	size += fieldsSize;

	return true;
}

bool DiskIO::Stream2Point(Grid::Point &point, char &state, int &size, char *stream, unsigned int space)
{
	size = 0;
	//数据状态
	if ( size + sizeof(char) > space ) return false;
	memcpy(&state, &stream[size], sizeof(char) );
	size += sizeof(char);
	//数据id
	if ( size + sizeof(bigint) > space ) return false;
	bigint id;
	memcpy(&id, &stream[size], sizeof(bigint) );
	if ( id != point.id ) return false;
	size += sizeof(bigint);
	//出边数
	if ( size + sizeof(int) > space ) return false;
	int iVal = 0;
	memcpy(&iVal, &stream[size], sizeof(int) );
	if ( 0 > iVal ) return false;
	size += sizeof(int);
	//id列表
	int i = 0;
	for (i = 0; i < iVal; i++ )
	{
		if ( size + sizeof(bigint) > space ) return false;
		memcpy(&id, &stream[size], sizeof(bigint) );
		if ( 0 >= id ) return false;
		point.out.Add(id);
		size += sizeof(bigint);
	}
	//入边数
	if ( size + sizeof(int) > space ) return false;
	memcpy(&iVal, &stream[size], sizeof(int) );
	if ( 0 > iVal ) return false;
	size += sizeof(int);
	//id列表
	for (i = 0; i < iVal; i++ )
	{
		if ( size + sizeof(bigint) > space ) return false;
		memcpy(&id, &stream[size], sizeof(bigint) );
		if ( 0 >= id ) return false;
		point.out.Add(id);
		size += sizeof(bigint);
	}

	//属性列表
	int fieldsSize;
	if ( !Stream2Fields(point.data, fieldsSize, &stream[size], space - size) ) return false;
	size += fieldsSize;

	return true;
}

//边数据压入字节流
bool DiskIO::Line2Stream(char *stream, Grid::Line &line, char state, int &size, unsigned int space)
{
	size = 0;
	//数据状态
	if ( size + sizeof(char) > space ) return false;
// 	if ( DiskIO::normal != state && DiskIO::del != state ) return false;
	memcpy(&stream[size], &state, sizeof(char) );
	size += sizeof(char);
	//数据id
// 	if ( 0 >= line.id ) return false;
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&stream[size], &line.id, sizeof(bigint) );
	size += sizeof(bigint);
	//起点id
// 	if ( 0 >= line.startId ) return false;
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&stream[size], &line.startId, sizeof(bigint) );
	size += sizeof(bigint);
	//终点id
// 	if ( 0 >= line.endId ) return false;
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&stream[size], &line.endId, sizeof(bigint) );
	size += sizeof(bigint);

	//属性列表
	int fieldsSize;
	if ( !Fields2Stream(&stream[size], line.data, fieldsSize, space - size) ) return false;
	size += fieldsSize;

	return true;
}

bool DiskIO::Stream2Line(Grid::Line &line, char &state, int &size, char *stream, unsigned int space)
{
	size = 0;
	//数据状态
	if ( size + sizeof(char) > space ) return false;
	memcpy(&state, &stream[size], sizeof(char) );
	size += sizeof(char);
	//数据id
	if ( size + sizeof(bigint) > space ) return false;
	bigint id;
	memcpy(&id, &stream[size], sizeof(bigint) );
	if ( id != line.id ) return false;
	size += sizeof(bigint);
	//起点id
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&line.startId, &stream[size], sizeof(bigint) );
	if ( 0 >= line.startId ) return false;
	size += sizeof(bigint);
	//终点id
	if ( size + sizeof(bigint) > space ) return false;
	memcpy(&line.endId, &stream[size], sizeof(bigint) );
	if ( 0 >= line.endId ) return false;
	size += sizeof(bigint);

	//属性列表
	int fieldsSize;
	if ( !Stream2Fields(line.data, fieldsSize, &stream[size], space - size) ) return false;
	size += fieldsSize;

	return true;
}

Grid::Result::Result DiskIO::CreatePoints(std::vector<Grid::Point*> &points)
{
	//参数检查,并按照文件分批
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
	DiskIO::AutoCloseFile autoCloseIdx(&idxFile);//自动关闭文件
	if ( mdk::File::success != idxFile.Open(mdk::File::add, mdk::File::assii) ) return Grid::Result::noPermissions;
	bigint idxPos = idxFile.Size();//记录索引开始位置
	bigint dataPos = 0;//数据开始位置
	int size = 0;//数据长度
	int idxSize = 0;//索引数据长度
	//遍历需要更新的文件
	std::map<bigint, std::vector<Grid::Point*> >::iterator it;
	for ( it = pointsMap.begin(); it != pointsMap.end(); it++ )
	{
		//定位数据文件
		mdk::File &dbFile = m_pointData[it->first];
		DiskIO::AutoCloseFile autoCloseDB(&dbFile);//自动关闭文件
		if ( mdk::File::success != dbFile.Open(mdk::File::add, mdk::File::assii) ) 
		{
			return Grid::Result::noPermissions;
		}
		dataPos = dbFile.Size();//记录数据开始位置
		//准备对一个文件写入
		std::vector<Grid::Point*> &points = it->second;
		count = points.size();
		idxSize = size = 0;
		for ( i = 0; i < count; i++ )
		{
			if ( idxSize + INDEX_SIZE > m_idxStreamSpace ) 
			{//索引流已经装满，先写入磁盘
				//写入数据
				if ( mdk::File::success != dbFile.Write(m_stream, size) ) return Grid::Result::noDiskSpace;
				//写入索引
				if ( mdk::File::success != idxFile.Write(&m_idxStream, idxSize) ) return Grid::Result::noDiskSpace;
				//继续打包剩余对象
				idxSize = size = 0;
				continue;
			}

			//打包数据流
			if ( !Point2Stream(&m_stream[size], *(points[i]), 
				DiskIO::normal, points[i]->diskSpace, m_streamSpace) ) 
			{//数据已经装满，先写入磁盘
				//写入数据
				if ( mdk::File::success != dbFile.Write(m_stream, size) ) return Grid::Result::noDiskSpace;
				//写入索引
				if ( mdk::File::success != idxFile.Write(&m_idxStream, idxSize) ) return Grid::Result::noDiskSpace;
				//继续打包剩余对象
				idxSize = size = 0;
				continue;
			}
			size += points[i]->diskSpace;//更新数据流长度
			//打包索引流
			memcpy(&m_idxStream[idxSize], &(points[i]->id), sizeof(bigint));//对象id
			idxSize += sizeof(bigint);//更新索引流长度
			memcpy(&m_idxStream[idxSize], &dataPos, sizeof(bigint));//数据开始位置
			idxSize += sizeof(bigint);//更新索引流长度
			memcpy(&m_idxStream[idxSize], &(points[i]->diskSpace), sizeof(int));//数据长度
			idxSize += sizeof(int);//更新索引流长度

			//保存索引和数据开始位置
			points[i]->SetIdxPos(idxPos);
			points[i]->SetDataPos(dataPos);
			//计算下一个对象在索引文件和数据文件中开始位置
			idxPos += INDEX_SIZE;
			dataPos += points[i]->diskSpace;
		}
		//写入数据
		if ( mdk::File::success != dbFile.Write(m_stream, size) ) return Grid::Result::noDiskSpace;
		//写入索引
		if ( mdk::File::success != idxFile.Write(m_idxStream, idxSize) ) return Grid::Result::noDiskSpace;
	}

	return Grid::Result::success;
}
