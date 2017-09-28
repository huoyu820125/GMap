#include "MsgSetPoint.h"
#include "Serialize.h"

namespace msg
{

MsgSetPoint::MsgSetPoint()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}

MsgSetPoint::~MsgSetPoint()
{
}

bool MsgSetPoint::Build( bool isResult )
{
	SetId(MsgId::setPoint, isResult);
	if (!FillTransmitParam()) return false;  // Tcp服务填写参数

 	if ( isResult ) return true; //回应无参数

	//请求参数
	std::vector<std::string> filter;
	if ( !Serialize::AddPoint(*this, m_point, true, filter) )	return false;//数据

	return true;
}

bool MsgSetPoint::Parse()
{
	if (!Buffer::Parse()) return false;

	if ( IsResult() ) return true;//回应无参数

	// 请求参数
	return Serialize::GetPoint(*this, m_point);//数据
}

}