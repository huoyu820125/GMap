#include "MsgDelPoint.h"

namespace msg
{

MsgDelPoint::MsgDelPoint()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}


MsgDelPoint::~MsgDelPoint()
{
}

bool MsgDelPoint::Build( bool isResult )
{
	SetId(MsgId::delPoint, isResult);
	if (!FillTransmitParam()) return false;  // Tcp服务填写参数

	if ( isResult ) return true; //回应不带参数

	//请求参数
	if ( !AddData(m_pointId) ) return false;

	return true;
}

bool MsgDelPoint::Parse()
{
	if (!Buffer::Parse()) return false;

	if ( IsResult() ) return true; // 回应不带参数

	// 请求参数
	if ( !GetData(m_pointId) ) return false;

	return true;
}
}