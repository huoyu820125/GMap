#include "MsgAddLine.h"
#include "Serialize.h"

namespace msg
{

MsgAddLine::MsgAddLine()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}


MsgAddLine::~MsgAddLine()
{
}

bool MsgAddLine::Build( bool isResult )
{
	SetId(MsgId::addLine, isResult);
	if (!FillTransmitParam()) return false;  // Tcp服务填写参数

	if ( isResult ) return true; //回应不带参数

	//请求参数
	std::vector<std::string> filter;
	if ( !Serialize::AddLine(*this, m_line, true, filter) ) return false;

	return true;
}

bool MsgAddLine::Parse()
{
	if (!Buffer::Parse()) return false;

	if (IsResult()) return true;//不需回应

	// 请求参数
	if ( !Serialize::GetLine(*this, m_line) ) return false;

	return true;
}

}
