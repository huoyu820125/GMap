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
	//ԭ�Ӳ���
	bool OnNewId(mdk::STNetHost &host, msg::Buffer &buffer);//ȡΨһ����id
	bool OnSetPoint(mdk::STNetHost &host, msg::Buffer &buffer);//���ö��㣬id>0���ö���,id<=0��������
	bool OnAddLineForPoint(mdk::STNetHost &host, msg::Buffer &buffer);//���������ӱ�
	bool OnDelLineForPoint(mdk::STNetHost &host, msg::Buffer &buffer);//ɾ������ı�
	bool OnGetPoint(mdk::STNetHost &host, msg::Buffer &buffer);//ȡ�ö���
	bool OnAddLine(mdk::STNetHost &host, msg::Buffer &buffer);//������
	bool OnDelLine(mdk::STNetHost &host, msg::Buffer &buffer);//ɾ����
	bool OnGetLine(mdk::STNetHost &host, msg::Buffer &buffer);//ȡ�ñ�

	//////////////////////////////////////////////////////////////////////////
	//���ϲ���
	bool OnDelPoint(mdk::STNetHost &host, msg::Buffer &buffer);//ɾ������
	bool OnSetMoveOpt(mdk::STNetHost &host, msg::Buffer &buffer);//����move
	bool OnMove( mdk::STNetHost &host, msg::Buffer &buffer );//�༶�ƶ�
	//////////////////////////////////////////////////////////////////////////
	//��������
	//�������ö���
	bool OnSetPoints( mdk::STNetHost &host, msg::Buffer &buffer );

private:
	mdk::Logger		m_log;
	mdk::ConfigFile	m_cfg;
	mdk::int32		m_nodeId;//�����
	GridStore	m_grid;//���ݿ�
	typedef struct MOVE_OPT//��ѯ����
	{
		bool							moveAllLine;//������·���ƶ� moveAllLine = trueʱ��selectLines��Ч
		std::vector<Grid::SELECT_LINE>	selectLines;//ѡ����ƶ�·��
		std::vector<Grid::FILTER>	pointFilter;//�յ�����ֶ�
		bool						selectAll;//ѡȡ�����ֶ�=trueʱgetFields��Ч
		std::vector<std::string>	getFields;//�յ��ȡ�ֶ�
		Grid::Action				act;//ִ�ж���

		std::vector<int64>			ids;//id�б�
	}MOVE_OPT;
	std::map<int, MOVE_OPT>		m_moveOpts;//�����϶�Ӧ��ѯ����
	mdk::Array<Grid::Point*>	m_endPoints;//�������������Ƶ��new������������

	//////////////////////////////////////////////////////////////////////////
	//������������
	std::map<int, std::vector<Grid::Point> >	m_points;//�����϶�Ӧ�������������б�
};

#endif //WORKER_H