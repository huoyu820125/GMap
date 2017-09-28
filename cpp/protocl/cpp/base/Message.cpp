#include "Message.h"
#include <cstdio>
#include <cstring>

namespace net
{

Message::Message(void)
{
}

Message::~Message(void)
{
}

void Message::BindSetting( unsigned char *buffer, int maxSize, char	byteOrder, unsigned char moudleId )
{
	Bind(buffer, maxSize);
	SetByteOrder((ByteOrder)byteOrder);
	m_moudleId = moudleId;
}

//报文头长度
int Message::HeaderSize()
{
	return sizeof(int) + sizeof(char) + sizeof(short) + sizeof(char);
}	

Message::operator unsigned char*()
{
	return Stream::operator unsigned char*();
}

Message::operator char*()
{
	return Stream::operator char*();
}

Message::operator void*()
{
	return Stream::operator void*();
}

//模块号
unsigned char Message::MoudleId()
{
	return m_moudleId;
}

//返回协议号
short Message::Id()
{
	return m_id;
}

//是回应报文
bool Message::IsResult()
{
	return 0 == m_isResult?false:true;
}

//设置id
void Message::SetId( short id, bool isResult )
{
	StartAdd();
	AddData(m_moudleId);
	AddData(id);
	AddData((char)(isResult?1:0));
}

//读取报文头
bool Message::ReadHeader()
{
	StartGet();
	if ( -1 == Size() ) return false;

	m_pos = sizeof(int);
	if ( !GetData(m_moudleId) ) return false;
	if ( !GetData(m_id) ) return false;
	if ( !GetData(m_isResult) ) return false;

	return true;
}

}
