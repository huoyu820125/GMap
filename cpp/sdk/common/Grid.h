#ifndef GRID_H
#define GRID_H

#include <map>
#include <string>
#include <vector>

#ifdef WIN32
typedef __int64				bigint;
#else
#include <sys/types.h>
typedef int64_t				bigint;
#endif

#define INDEX_SIZE (sizeof(bigint) + sizeof(bigint) + sizeof(int))//索引长度：数据id+数据文件中位置+数据长度

namespace Grid
{
	//存储限制
	namespace Limit
	{
		enum Limit
		{
			/*
				顶点磁盘占用：
					835byte：
						数据状态(char) + 数据id(int64) + 出边数量(int) + 入边数量(int) + 字段数量(short)
						+ max字段数(10) * 
						(字段名长度(short) + 字段名(16 + 1) + 字段类型(char) + 字段值长度(int) + 字段值(64))
					max18963byte：
						数据状态(char) + 数据id(int64) + 出边数量(int) + 入边数量(int) + 字段数量(short)
						+ max字段数(64) * 
						(字段名长度(short) + 字段名(32 + 1) + 字段类型(char) + 字段值长度(int) + 字段值(256))

				边max占用磁盘18971byte：
					195byte：
						数据状态(char) + 数据id(int64) + 起点id(int64) + 终点id(int64) + 字段数量(short)
						+ max字段数(3) * 
						(字段名长度(short) + 字段名(16 + 1) + 字段类型(char) + 字段值长度(int) + 字段值(32))
					max18971byte：
						数据状态(char) + 数据id(int64) + 起点id(int64) + 终点id(int64) + 字段数量(short)
						+ max字段数(64) * 
						(字段名长度(short) + 字段名(32 + 1) + 字段类型(char) + 字段值长度(int) + 字段值(256))

				10亿顶点+1000亿条边，max占用磁盘byte：
					min:
						835*10亿+195*1000亿+8*1000亿+8*1000亿
						16T=16224G=778G+15446G
						单个数据文件2.6G= 16224G / 3节点 / 8核 / 256文件数
						0.88020833333333333333333333333333
					max:
						18963*10亿+18971*1000亿+8*1000亿+8*1000亿
						1785084G=1767424G+17660G
			*/
			maxFieldCount = 64,//顶点和边允许带有的最大字段数
			maxFieldNameSize = 32,//字段名最大长度
			maxFieldSize = 256,//字段最大长度
			maxDBShardCount = 256,//数据文件最大分片数
			maxFilter = 16,//过滤条件最大个数
			maxMoveCount = 10,//移动最大经过边数
			maxObjectCount = 1000000,//批量处理最大对象数100万
		};
	}

	namespace Result
	{
		enum Result
		{
			success = 0,
			paramError = 1,	//参数错误
			notOwner = 2,//节点不是数据拥有者
			noData = 3,//无数据
			startError = 4,	//起点错误
			endError = 5,	//终点错误
			fileError = 6,//文件错误
			noIdSource = 7,//无可用id
			tooManyField = 8,//字段太多
			tooBigFieldName = 9,//字段名太长
			tooBigField = 10,//字段长度太大

			//DiskIO错误
			noDataDir = 11,//没有数据目录
			noPermissions = 12,//无文件或文件夹权限
			noFile = 13,//文件不存在
			noDiskSpace = 14,//磁盘不足
			dataDamage = 15,//数据损坏
			noMemery = 16,//内存不足
			idxDamage = 17,//索引数据损坏
			idxError = 18,//索引错误

		};
		char* Reason(Result result);
	}

	//类型定义
	enum DataType
	{
		undef = 0,//未定义类型
		int8 = 1,//8字节整型 char byte bool
		int16 = 2,//2字节整型 short
		int32 = 3,//4字节整型 int
		int64 = 4,//8字节整型 bigint
		str = 5,//字符串
		date = 6,//日期
	};

	//字段
	typedef struct FIELD
	{
		DataType		type;
		bigint			value;//保存整型数据
		char			data[Limit::maxFieldSize+1];//保存字符串，序列化类型数据
		int				size;//数据长度
	}FIELD;

	//move方法，对顶点的操作
	enum Action
	{
		get = 0,//取得顶点数据(多次move后的终点)
		count = 1,//统计顶点数量(多次move后的终点)
		del = 2,//删除顶点(1次move终点)
		unlink = 3,//断开连接(1次move)
	};

	//比较方式
	enum CmpMode
	{
		left = -2, //<
		leftEquals = -1, //<=
		equals = 0,//=
		rightEquals = 1,//>
		right = 2,//>=
		unequals = 3,//!=
	};
	//选择
	typedef struct FILTER
	{
		char			cmpMode;//比较方式，enum CmpMode
		std::string		fieldName;//字段名
		unsigned int	hashValue;//hash值
		FIELD			field;//字段
	}FILTER;
	//move操作1次移动选择的边，匹配条件
	typedef struct SELECT_LINE
	{
		bool moveOut;//选出边move
		bool moveAllLine;//从moveOut指定的方向上所有路线移动，moveAllLine = true时，selectField无效
		std::vector<Grid::FILTER> selectField;//选匹配字段的边
	}SELECT_LINE;

	//节点(nodeId)是数据的拥有者（数据存储在节点(nodeId)上）
	bool IsOwner(bigint dataId, int nodeId, int nodeCount);
	//寻找数据保存在哪个节点上
	int FindNodeId(bigint dataId, int nodeCount);

	//远程对象
	typedef struct REMOTE_DATA{
		int lineIndex;
		std::vector<bigint> lineIds;
		std::vector<bigint> pointIds;
	}REMOTE_DATA;

}

#endif //GRID_H
