#include "MsgGetPoint.h"
#include "Serialize.h"

namespace msg
{

MsgGetPoint::MsgGetPoint()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}


MsgGetPoint::~MsgGetPoint()
{
}

bool MsgGetPoint::Build( bool isResult )
{
	SetId(MsgId::getPoint, isResult);
	if (!FillTransmitParam()) return false;  // Tcp服务填写参数

	//请求参数
	if ( !isResult )
	{
		if ( !AddData(m_pointId) ) return false;
		return true;
	}

	//回应参数
	if ( ResultCode::success != m_code ) return true;
	if ( !Serialize::AddPoint(*this, m_point, true, m_getFields) )return false;

	return true;
}

bool MsgGetPoint::Parse()
{
	if (!Buffer::Parse()) return false;

	// 请求参数
	if ( !IsResult() )
	{
		if ( !GetData(m_pointId) ) return false;
		return true;
	}

	// 回应参数
	if ( ResultCode::success != m_code ) return true;
	if ( !Serialize::GetPoint(*this, m_point) ) return false;

	return true;
}

}
