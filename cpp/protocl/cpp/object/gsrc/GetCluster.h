#ifndef GET_CLUSTER_H
#define GET_CLUSTER_H

#include "../../Buffer.h"
#include <map>
#include <vector>
#include "common/Common.h"

namespace msg
{

//取集群信息
class GetCluster : public Buffer
{
public:
	//请求参数
	Moudle::Moudle	m_serviceType; //要取的服务类型，int32   
	int				m_start;      //从第start个服务开始
	//回应参数

	//map<线路, vector<结点>>
	std::map<NetLine::NetLine, std::vector<NODE> > m_cluster;

public:
	GetCluster();
	virtual ~GetCluster();
	bool Build( bool isResult = false);//构造报文，发送
	bool Parse();//解析报文，接收

private:
	unsigned char m_buffer[MAX_MSG_SIZE];
};


}
#endif //GET_CLUSTER_H