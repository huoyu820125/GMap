#include "Client.h"

Client::Client(void)
{
	net::Socket::SocketInit();
	m_svrIp = "";
	m_svrPort = 0;
	m_isConnected = false;
	m_lastConnectFailed = 0;
}

Client::~Client(void)
{
}

void Client::SetService( int nodeId, const std::string &ip, int port )
{
	m_nodeId = nodeId;
	m_svrIp = ip;
	m_svrPort = port;
}

bool Client::IsReady()
{
	if ( !m_isConnected )
	{
		if ( 300 > time(NULL) - m_lastConnectFailed ) return false;
	}
	if ( Connect() ) return true;
	m_lastConnectFailed = time(NULL);

	return false;
}

bool Client::Connect()
{
	if ( m_isConnected ) return true;

	if ( !m_svr.Init(net::Socket::tcp) ) return false;
	if ( !m_svr.Connect(m_svrIp.c_str(), m_svrPort) ) return false;
	m_svr.SetSockMode(true);
	m_svr.SetNoDelay(true);
	m_isConnected = true;

	return true;
}

void Client::Close()
{
	if ( !m_isConnected ) return;
	m_svr.Close();
	m_isConnected = false;
}

CallResult Client::Query(msg::Buffer *msg, int millSecond)
{
	if ( !msg->Build() ) return Call::Error(ResultCode::paramError, "���Ĺ���ʧ��");
	CallResult result = Send(*msg, msg->Size());
	if ( !result.isSuccess ) return result;
	return ReadMsg(msg, millSecond);
}

CallResult Client::Send(char *msg, int size)
{
	if ( !Connect() ) 
	{
		char nosvr[256];
		sprintf( nosvr, "ͼ���ݿ�ڵ�(%d)%s %d:�޷���", m_nodeId, m_svrIp.c_str(), m_svrPort );
		return Call::Error(ResultCode::netError, nosvr);
	}

	if ( 0 > m_svr.Send(msg, size) )
	{
		Close();//����ʧ�ܱ���Ͽ����ӣ������´�����ʱ�����յ��ϴ�δ���յ�����
		//�����������ܶ�ʧ������1��
		if ( !Connect() )
		{
			char nosvr[256];
			sprintf( nosvr, "ͼ���ݿ�ڵ�(%d)%s %d:�޷���", m_nodeId, m_svrIp.c_str(), m_svrPort );
			return Call::Error(ResultCode::netError, nosvr);
		}

		if ( 0 > m_svr.Send(msg, size) ) 
		{
			Close();//����ʧ�ܱ���Ͽ����ӣ������´�����ʱ�����յ��ϴ�δ���յ�����
			char nosvr[256];
			sprintf( nosvr, "ͼ���ݿ�ڵ�(%d)%s %d:�޷���", m_nodeId, m_svrIp.c_str(), m_svrPort );
			return Call::Error(ResultCode::netError, nosvr);
		}
	}

	return Call::Success();
}

CallResult Client::ReadMsg(msg::Buffer *msg, int millSecond)
{
	int ret;
	int size = msg->HeaderSize();
	char *buf = *msg;
	int pos = 0;
	while ( 0 < size )
	{
		ret = m_svr.Receive(&buf[pos], size, true, millSecond);
		if ( 0 > ret )
		{
			Close();//����ʧ�ܱ���Ͽ����ӣ������´�����ʱ�����յ��ϴ�δ���յ�����
			if ( net::Socket::seTimeOut == ret ) 
			{
				char busy[256];
				sprintf( busy, "ͼ���ݿ�ڵ�(%d)%s %d:����æ", m_nodeId, m_svrIp.c_str(), m_svrPort );
				return Call::Error(ResultCode::netError, busy);
			}
			else 
			{
				char nosvr[256];
				sprintf( nosvr, "ͼ���ݿ�ڵ�(%d)%s %d:�޷���", m_nodeId, m_svrIp.c_str(), m_svrPort );
				return Call::Error(ResultCode::netError, nosvr);
			}
		}
		size -= ret;
		pos += ret;
	}
	if ( !msg->ReadHeader() ) //����ʧ�ܱ���Ͽ����ӣ���������������ݣ��ƻ��´�����
	{
		Close();
		return Call::Error(ResultCode::msgError, "����˱���ͷ�쳣");
	}
	size = msg->Size();
	pos = 0;
	while ( 0 < size )
	{
		ret = m_svr.Receive(&buf[pos], size, false, millSecond);
		if ( 0 > ret )//����ʧ�ܱ���Ͽ����ӣ������´�����ʱ�����յ��ϴ�δ���յ�����
		{
			Close();
			if ( net::Socket::seTimeOut == ret )
			{
				char busy[256];
				sprintf( busy, "ͼ���ݿ�ڵ�(%d)%s %d:����æ", m_nodeId, m_svrIp.c_str(), m_svrPort );
				return Call::Error(ResultCode::netError, busy);
			}
			else
			{
				char nosvr[256];
				sprintf( nosvr, "ͼ���ݿ�ڵ�(%d)%s %d:�޷���", m_nodeId, m_svrIp.c_str(), m_svrPort );
				return Call::Error(ResultCode::netError, nosvr);
			}
		}
		size -= ret;
		pos += ret;
	}

	if ( !msg->Parse() ) //����ʧ�ܱ���Ͽ����ӣ���������������ݣ��ƻ��´�����
	{
		Close();
		return Call::Error(ResultCode::msgError, "����˱��ĸ�ʽ�쳣");
	}
	if ( ResultCode::success != msg->m_code ) return Call::Error(msg->m_code, msg->m_reason);
	
	return Call::Success();
}

bool Client::IsEnd()
{
	char buf[256];
	int count = m_svr.Receive(buf, 1, true, 0, 1);
	if ( 1 == count ) return false;

	return true;
}
