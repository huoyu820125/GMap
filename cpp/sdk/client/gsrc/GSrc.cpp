#include "GSrc.h"

GSrc::GSrc(void)
{
}


GSrc::~GSrc(void)
{
}

//»°∑˛ŒÒ≈‰÷√
CallResult GSrc::GetServices(std::map<NetLine::NetLine, std::vector<NODE> > &cluster, Moudle::Moudle svrType)
{
	msg::GetCluster msg;
	msg.m_start = 1;
	while ( true )
	{
		msg.m_serviceType = svrType;
		CallResult result = Query(&msg, 10);
		if ( !result.isSuccess ) return result;

		if ( 0 == msg.m_cluster.size() ) break;
		std::map<NetLine::NetLine, std::vector<NODE> >::iterator itLine = msg.m_cluster.begin();
		for ( ; itLine != msg.m_cluster.end(); itLine++ )
		{
			int i = 0;
			std::vector<NODE> &nodes = itLine->second;
			for ( i = 0; i < nodes.size(); i ++ )
			{
				cluster[itLine->first].push_back(nodes[i]);
			}
			msg.m_start += nodes.size();
		}
		
	}

	return Call::Success();
}

CallResult GSrc::GetClientId(int64 &clientId)
{
	msg::MsgGetClientId msg;
	CallResult result = Query(&msg, 10);
	if ( !result.isSuccess ) return result;
	clientId = msg.m_clientId;

	return Call::Success();
}
