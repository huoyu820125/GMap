#include "GetCluster.h"


namespace msg
{

GetCluster::GetCluster()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gsrc);
}


GetCluster::~GetCluster()
{
}

bool GetCluster::Build( bool isResult )
{
	SetId(MsgId::getCluster, isResult);
	if ( !FillTransmitParam() ) return false;//Tcp服务填写参数
	//请求参数
	if ( !AddData((int)m_serviceType) ) return false;//取什么模块的服务
	if ( !AddData(m_start) ) return false;//从第start个服务开始


	//回应参数
	if (!isResult || ResultCode::success != m_code) return true;

	//map<线路, vector<结点>>
	if ( 4 < m_cluster.size() ) return false;

	if ( !AddData((char)m_cluster.size()) ) return false;
	std::map<NetLine::NetLine, std::vector<NODE> >::iterator it = m_cluster.begin();
	for ( ; it != m_cluster.end(); it++ ) 
	{
		if ( !AddData((unsigned char)it->first) ) return false;
		int i = 0;
		std::vector<NODE> &nodes = it->second;
		if ( 0 == nodes.size() ) return false;//禁止空数组被传递

		if ( !AddData((unsigned char)nodes.size()) ) return false;
		for (i = 0; i < nodes.size(); i++)
		{
			if ( !AddData(nodes[i].nodeId) ) return false;
			if ( !AddData(nodes[i].ip) ) return false;
			if ( !AddData(nodes[i].port) ) return false;
		}
	}
	return true;
}

bool GetCluster::Parse()
{
 	if ( !Buffer::Parse() ) return false;
	//请求参数
	int val;
	if ( !GetData(val) ) return false;//取什么模块的服务
	m_serviceType = (Moudle::Moudle)val;
	if ( !GetData(m_start) ) return false;//从第start个服务开始


	//回应参数
	if (!IsResult() || ResultCode::success != m_code) return true;

	//map<线路, vector<结点>>
	char count;
	if ( !GetData(count) ) return false;
	if ( 0 > count || 4 < count ) return false;

	std::map<NetLine::NetLine, std::vector<NODE> >::iterator it = m_cluster.begin();
	int i = 0;
	NetLine::NetLine line;
	unsigned char nodeCount;
	NODE node;
	unsigned char value;
	m_cluster.clear();
	for ( i = 0; i < count; i++ ) 
	{
		if ( !GetData(value) ) return false;
		line = (NetLine::NetLine)value;
		int j = 0;
		if ( !GetData(nodeCount) ) return false;
		if ( 0 >= nodeCount ) return false;

		for (j = 0; j < nodeCount; j++)
		{
			if ( !GetData(node.nodeId) ) return false;
			if ( !GetData(node.ip) ) return false;
			if ( !GetData(node.port) ) return false;
			m_cluster[line].push_back(node);
		}
	}

	return true;
}

}
