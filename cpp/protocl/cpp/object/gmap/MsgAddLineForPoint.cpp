#include "MsgAddLineForPoint.h"
#include "Serialize.h"

namespace msg
{

MsgAddLineForPoint::MsgAddLineForPoint()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}


MsgAddLineForPoint::~MsgAddLineForPoint()
{
}

bool MsgAddLineForPoint::Build( bool isResult )
{
	SetId(MsgId::addLineForPoint, isResult);
	if (!FillTransmitParam()) return false;  // Tcp服务填写参数

	if ( isResult ) return true; //回应不带参数

	//请求参数
	if ( !AddData(m_pointId) ) return false;
	if ( !AddData(m_lineId) ) return false;
	if ( !AddData(m_isOut) ) return false;

	return true;
}

bool MsgAddLineForPoint::Parse()
{
	if (!Buffer::Parse()) return false;

	if ( IsResult() ) return true;//回应不带参数

	//请求参数
	if ( !GetData(m_pointId) ) return false;
	if ( !GetData(m_lineId) ) return false;
	if ( !GetData(m_isOut) ) return false;

	return true;
}

}