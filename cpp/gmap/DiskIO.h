#ifndef DISKIO_H
#define DISKIO_H

#include "mdk_ex/File.h"

#include <vector>
#include <map>
#include "common/struct/Point.h"
#include "common/struct/Line.h"


//磁盘io
//数据持久化和读取
/*
所有顶点与边的操作必须使用id作为索引
不支持属性索引
如果上层需要的数据如果只有属性，没有id，但是又需要到磁盘获取，
必须确定id范围，通过id将数据拿到内存，然后再在内存中匹配对象属性
不属于此类的作用范围

文件存储结构
存储文件分类
顶点索引文件：存储顶点id对应顶点数据在数据文件中起始位置
顶数据引文件：存储顶点数据
边索引文件：存储边id对应边数据在数据文件中起始位置
边数据文件：存储边数据
索引文件名：point/line+编号(1~512)+.idx
数据文件名：point/line+编号(1~512)+.db

索引文件格式：顶点or边id(8byte) + 数据在db文件中的开始位置(8byte) + 数据长度(4byte)
数据文件格式：数据状态(1byte)0正常，1删除 + 数据(nbyte)

1000万顶点，每个顶点2k，分成512个数据文件存储，每个文件大小38M
2000万条边，每条边2k，分成512个数据文件存储，每个文件大小76M
*/
class GridStore;
class DiskIO
{
public:
	DiskIO();
	virtual ~DiskIO();
	void Init(GridStore *pStore);
	std::string DataDir();
	const char* CheckDataDir();//检查数据目录
	//方法返回错误码
	enum Error
	{
		noDataDir = 1,//没有数据目录
		noPermissions = 2,//无文件或文件夹权限
		noFile = 3,//文件不存在
		errorParam = 4,//参数错误
		noDiskSpace = 5,//磁盘不足
		dataDamage = 6,//数据损坏
		noData = 7,//没有数据
	};
	const char* LoadPoints(std::map<bigint, Grid::Point*> &points);//读取所有顶点
	const char* LoadLines(std::map<bigint, Grid::Line*> &points);//读取所有边

public:
	Grid::Result::Result SavePoint(Grid::Point &point);//保存顶点
	Grid::Result::Result DelPoint(bigint pointId, bigint dataPos);//删除顶点
	Grid::Result::Result SaveLine(Grid::Line &line);//保存边
	Grid::Result::Result DelLine(bigint lineId, bigint dataPos);//删除边
	//////////////////////////////////////////////////////////////////////////
	//批量操作
	//创建一批顶点
	Grid::Result::Result CreatePoints(std::vector<Grid::Point*> &points);

protected:
	//数据状态
	enum DataState
	{
		normal = 0,//正常
		del = 1,//删除
	};

	class AutoCloseFile
	{
	public:
		AutoCloseFile( mdk::File *pFile );
		~AutoCloseFile();

		void Detach();//分离文件，文件将不会自动关闭

	private:
		mdk::File *m_pFile;
	};

	const char* ReadPointFile(mdk::File &idxFile, std::map<bigint, Grid::Point*> &points);//读取文件中所有顶点
	const char* ReadLineFile(mdk::File &idxFile, std::map<bigint, Grid::Line*> &points);//读取文件中所有边
	Grid::Result::Result ReadPoint(const bigint &dataPos, Grid::Point &point);
	Grid::Result::Result ReadLine(const bigint &dataPos, Grid::Line &line);
	//写入索引，如果isUpdate = false则将索引位置保存道idxPos
	Grid::Result::Result SaveIndex(mdk::File &idxFile, bool isUpdated, bigint &idxPos, bigint id, const bigint &dataPos, int size);
	/*
		开始保存数据
		isUpdated = false表示数据是第一次创建，否则表示修改已有数据,做更新操作

		创建数据
			直接将文件游标移动到文件末尾，准备写入，并将开始位置保存到dataPos
		修改数据
			检查数据长度是否超过旧数据长度
			如果超过，则将旧数据设置为删除，并将文件游标移动到文件末尾,将dataMoved设置为true，准备写入输入
			否则将文件游标移动到数据开始位置，准备写入
	*/
	Grid::Result::Result SaveDataStart(mdk::File &dbFile, bool isUpdated, bool &dataMoved, bigint &dataPos, int dataSize);
	//开始读取数据，得到数据长度
	Grid::Result::Result ReadDataStart(mdk::File &dbFile, const bigint &dataPos, int &size);

	//保存数据到文件,如果是写入末尾，则将数据位置保存到dataPos
	Grid::Result::Result SaveData(mdk::File &dbFile, bigint &dataPos, char *data, int size, int oldSize, bool isCreate);

	//////////////////////////////////////////////////////////////////////////
	//数据&字节流之间转换方法
	/*
		只有正确的数据才停留在内存中
		所以进入持久层代码的数据，一定是进过业务检查的，所以数据->字节流的转换方法不做业务检查，只检查数组越界（数据超大）
		文件中读出的数据，必须经过业务检查，才可以进入内存，所以字节流->数据的方法，不仅仅要检查数组越界，还要做业务检查
	*/
	//属性列表压入字节流
	bool Fields2Stream(char *stream, mdk::Map &fields, int &size, unsigned int space);
	//从字节流获取属性列表
	bool Stream2Fields(mdk::Map &fields, int &size, char *stream, unsigned int space);
	//顶点数据压入字节流
	bool Point2Stream(char *stream, Grid::Point &point, char state, int &size, unsigned int space);
	//从字节流获取顶点数据
	bool Stream2Point(Grid::Point &point, char &state, int &size, char *stream, unsigned int space);
	//边数据压入字节流
	bool Line2Stream(char *stream, Grid::Line &line, char state, int &size, unsigned int space);
	//从字节流获取边数据
	bool Stream2Line(Grid::Line &line, char &state, int &size, char *stream, unsigned int space);


//////////////////////////////////////////////////////////////////////////
//测试方法
public:
	Grid::Result::Result ReadPoint(Grid::Point &point);
	Grid::Result::Result ReadLine(Grid::Line &line);
private:
	Grid::Result::Result ReadIndex(mdk::File &ideFile, bigint id, bigint &idxPos, bigint &dataPos);

private:
	std::string	m_dataDir;//数据目录
	mdk::File m_pointIdx;	//顶点索引文件
	mdk::File m_pointData[Grid::Limit::maxDBShardCount];	//顶点数据文件
	mdk::File m_lineIdx;	//边索引文件
	mdk::File m_lineData[Grid::Limit::maxDBShardCount];	//边数据文件
	GridStore *m_pStore;

	char	*m_stream;//数据对象转成流写入磁盘，和从磁盘读取流时候存储数据(复用)
	int		m_streamSpace;//空间大小
	char	*m_idxStream;//数据索引转成流写入磁盘，和从磁盘读取流时候存储数据(复用)
	int		m_idxStreamSpace;//数据索引空间大小

};

#endif //DISKIO_H
