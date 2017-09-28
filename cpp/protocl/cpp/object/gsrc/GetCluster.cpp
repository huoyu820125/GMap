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
	if ( !FillTransmitParam() ) return false;//Tcp������д����
	//�������
	if ( !AddData((int)m_serviceType) ) return false;//ȡʲôģ��ķ���
	if ( !AddData(m_start) ) return false;//�ӵ�start������ʼ


	//��Ӧ����
	if (!isResult || ResultCode::success != m_code) return true;

	//map<��·, vector<���>>
	if ( 4 < m_cluster.size() ) return false;

	if ( !AddData((char)m_cluster.size()) ) return false;
	std::map<NetLine::NetLine, std::vector<NODE> >::iterator it = m_cluster.begin();
	for ( ; it != m_cluster.end(); it++ ) 
	{
		if ( !AddData((unsigned char)it->first) ) return false;
		int i = 0;
		std::vector<NODE> &nodes = it->second;
		if ( 0 == nodes.size() ) return false;//��ֹ�����鱻����

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
	//�������
	int val;
	if ( !GetData(val) ) return false;//ȡʲôģ��ķ���
	m_serviceType = (Moudle::Moudle)val;
	if ( !GetData(m_start) ) return false;//�ӵ�start������ʼ


	//��Ӧ����
	if (!IsResult() || ResultCode::success != m_code) return true;

	//map<��·, vector<���>>
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
