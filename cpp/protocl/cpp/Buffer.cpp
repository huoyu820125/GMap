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
	//将数据部分保存下来
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
	if ( !AddData(m_connectId) ) return false;//链接Id	Tcp服务填写（回应必填）
	if ( !AddData(m_objectId) ) return false;//用户id(TCP接入服填写)
	if ( !AddData(m_ip[0]) ) return false;//用户id(TCP接入服填写)
	if ( !AddData(m_ip[1]) ) return false;//用户id(TCP接入服填写)
	if ( !AddData(m_ip[2]) ) return false;//用户id(TCP接入服填写)
	if ( !AddData(m_ip[3]) ) return false;//用户id(TCP接入服填写)
	if ( !AddData((char)m_code) ) return false;//结果码 0成功 >0失败
	if ( ResultCode::success != m_code )
	{
		if ( !AddData(m_reason) ) return false;//失败原因(成功时不填写)
	}

	return true;
}

bool Buffer::Parse()
{
	ReadHeader();
	if ( !GetData(m_connectId) ) return false;//链接Id	Tcp服务填写（回应必填）
	if ( !GetData(m_objectId) ) return false;//用户id(TCP接入服填写)
	if ( !GetData(m_ip[0]) ) return false;//用户id(TCP接入服填写)
	if ( !GetData(m_ip[1]) ) return false;//用户id(TCP接入服填写)
	if ( !GetData(m_ip[2]) ) return false;//用户id(TCP接入服填写)
	if ( !GetData(m_ip[3]) ) return false;//用户id(TCP接入服填写)
	char val;
	if ( !GetData(val) ) return false;//结果码 0成功 >0失败
	m_code = (ResultCode::ResultCode) val;
	if ( ResultCode::success != m_code )
	{
		if ( !GetData(m_reason) ) return false;//失败原因(成功时不填写)
	}

	return true;
}

}
