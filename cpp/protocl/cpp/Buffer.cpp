#include "Buffer.h"
#include <cstring>

namespace msg
{

Buffer::Buffer()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, 0);
	m_code = ResultCode::success;
}

Buffer::~Buffer()
{
}

bool Buffer::Build( bool isResult )
{
	//Header     + m_connectId   + m_objectId  + m_ip        + m_code
	int headerSize = HeaderSize() + sizeof(int64) + sizeof(int) + sizeof(int) + sizeof(char);
	//�����ݲ��ֱ�������
	char buf[10240];
	int dataSize = Size() - headerSize;
	if ( dataSize < 0 ) return false;
	StartAdd();
	memcpy(buf, &m_buffer[headerSize], dataSize );

	m_size = HeaderSize();
	if ( !FillTransmitParam() ) return false;
	memcpy(&m_buffer[m_size], buf, dataSize );
	m_size += dataSize;
	return true;
}

bool Buffer::FillTransmitParam()
{
	if ( !AddData(m_connectId) ) return false;//����Id	Tcp������д����Ӧ���
	if ( !AddData(m_objectId) ) return false;//�û�id(TCP�������д)
	if ( !AddData(m_ip[0]) ) return false;//�û�id(TCP�������д)
	if ( !AddData(m_ip[1]) ) return false;//�û�id(TCP�������д)
	if ( !AddData(m_ip[2]) ) return false;//�û�id(TCP�������д)
	if ( !AddData(m_ip[3]) ) return false;//�û�id(TCP�������д)
	if ( !AddData((char)m_code) ) return false;//����� 0�ɹ� >0ʧ��
	if ( ResultCode::success != m_code )
	{
		if ( !AddData(m_reason) ) return false;//ʧ��ԭ��(�ɹ�ʱ����д)
	}

	return true;
}

bool Buffer::Parse()
{
	ReadHeader();
	if ( !GetData(m_connectId) ) return false;//����Id	Tcp������д����Ӧ���
	if ( !GetData(m_objectId) ) return false;//�û�id(TCP�������д)
	if ( !GetData(m_ip[0]) ) return false;//�û�id(TCP�������д)
	if ( !GetData(m_ip[1]) ) return false;//�û�id(TCP�������д)
	if ( !GetData(m_ip[2]) ) return false;//�û�id(TCP�������д)
	if ( !GetData(m_ip[3]) ) return false;//�û�id(TCP�������д)
	char val;
	if ( !GetData(val) ) return false;//����� 0�ɹ� >0ʧ��
	m_code = (ResultCode::ResultCode) val;
	if ( ResultCode::success != m_code )
	{
		if ( !GetData(m_reason) ) return false;//ʧ��ԭ��(�ɹ�ʱ����д)
	}

	return true;
}

}
