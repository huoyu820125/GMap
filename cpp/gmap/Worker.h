#ifndef WORKER_H
#define WORKER_H

#include "mdk/include/frame/netserver/STNetServer.h"
#include "mdk/include/mdk/Logger.h"
#include "mdk/include/mdk/ConfigFile.h"
#include "mdk/include/mdk/Lock.h"
#include "mdk/include/mdk/Executor.h"
#include <vector>
#include <string>
#include "protocl/cpp/Buffer.h"
#include "GridStore.h"
#include "common/Common.h"
#include "mdk_ex/container/Array.h"


class Worker : public mdk::STNetServer
{
	friend int main(int argc, char* argv[]);
public:
	Worker(std::string cfgname);
	virtual ~Worker(void);

	mdk::Logger& Logger();
	virtual void OnConnect(mdk::STNetHost &host);
	virtual void OnCloseConnect(mdk::STNetHost &host);
	virtual void OnMsg(mdk::STNetHost &host);

	//////////////////////////////////////////////////////////////////////////
	//原子操作
	bool OnNewId(mdk::STNetHost &host, msg::Buffer &buffer);//取唯一对象id
	bool OnSetPoint(mdk::STNetHost &host, msg::Buffer &buffer);//设置顶点，id>0设置顶点,id<=0创建顶点
	bool OnAddLineForPoint(mdk::STNetHost &host, msg::Buffer &buffer);//给顶点增加边
	bool OnDelLineForPoint(mdk::STNetHost &host, msg::Buffer &buffer);//删除顶点的边
	bool OnGetPoint(mdk::STNetHost &host, msg::Buffer &buffer);//取得顶点
	bool OnAddLine(mdk::STNetHost &host, msg::Buffer &buffer);//创建边
	bool OnDelLine(mdk::STNetHost &host, msg::Buffer &buffer);//删除边
	bool OnGetLine(mdk::STNetHost &host, msg::Buffer &buffer);//取得边

	//////////////////////////////////////////////////////////////////////////
	//复合操作
	bool OnDelPoint(mdk::STNetHost &host, msg::Buffer &buffer);//删除顶点
	bool OnSetMoveOpt(mdk::STNetHost &host, msg::Buffer &buffer);//设置move
	bool OnMove( mdk::STNetHost &host, msg::Buffer &buffer );//多级移动
	//////////////////////////////////////////////////////////////////////////
	//批量操作
	//批量设置顶点
	bool OnSetPoints( mdk::STNetHost &host, msg::Buffer &buffer );

private:
	mdk::Logger		m_log;
	mdk::ConfigFile	m_cfg;
	mdk::int32		m_nodeId;//结点编号
	GridStore	m_grid;//数据库
	typedef struct MOVE_OPT//查询设置
	{
		bool							moveAllLine;//从所有路线移动 moveAllLine = true时，selectLines无效
		std::vector<Grid::SELECT_LINE>	selectLines;//选择的移动路线
		std::vector<Grid::FILTER>	pointFilter;//终点过滤字段
		bool						selectAll;//选取所有字段=true时getFields无效
		std::vector<std::string>	getFields;//终点截取字段
		Grid::Action				act;//执行动作

		std::vector<int64>			ids;//id列表
	}MOVE_OPT;
	std::map<int, MOVE_OPT>		m_moveOpts;//连接上对应查询设置
	mdk::Array<Grid::Point*>	m_endPoints;//检索结果，避免频繁new操作消耗性能

	//////////////////////////////////////////////////////////////////////////
	//批量操作参数
	std::map<int, std::vector<Grid::Point> >	m_points;//连接上对应批量操作顶点列表
};

#endif //WORKER_H