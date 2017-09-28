#ifndef PROTOCL_H
#define PROTOCL_H

#define MAX_MSG_SIZE 10240	//报文最大长度
#define BYTE_ORDER	net::smallOrder	//字节序

//模块（服务）定义
namespace Moudle
{
	enum Moudle
	{
		all = 0,				//所有模块
		gsrc = 1,		//集群配置模块
		gmap = 2,			//图数据库
	};
}

//报文Id
namespace MsgId
{
	enum MsgId
	{
		//集群配置模块
		getCluster = 1001,//取集群信息
		getClientId = 1002,//取唯一client id

		//图数据库主服务
		newId = 1001,//产生一个唯一id
		setPoint = 1002,//设置顶点
		addLineForPoint = 1003,//给顶点添加边
		delLineForPoint = 1004,//删除顶点的边
		getPoint = 1005,//取得顶点
		addLine = 1006,//创建边
		delLine = 1007,//删除边
		getLine = 1008,//取得边
		delPoint = 1009,//删除顶点
		setMoveOpt = 1010,//设置move参数，move2 move1 filterPoint共用
		move = 1011,//多级移动
		points = 1012,//找到的顶点
		moveMids = 1013,//move操作中当前节点无数据的对象id
		//批量操作
		setPoints = 1014,//批量设置顶点
		linkPoints = 1015,//批量连接顶点
		delPoints = 1016,//批量删除顶点
	};
}

namespace ResultCode
{
	enum ResultCode
	{
		success = 0, //成功
		paramError = 1,//参数错误
		netError = 2,//网络错误
		msgError = 3,//服务端报文错误：无法解析，非预期的报文
		refuse = 4,//拒绝
	};
};

#endif //PROTOCL_H