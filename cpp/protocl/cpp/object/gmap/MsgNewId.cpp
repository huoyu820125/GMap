#include "MsgNewId.h"

namespace msg
{

MsgNewId::MsgNewId()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
	m_count = 1;//产生id数量
}


MsgNewId::~MsgNewId()
{
}

bool MsgNewId::Build( bool isResult )
{
	SetId(MsgId::newId, isResult);
	if (!FillTransmitParam()) return false;  // Tcp服务填写参数

	if ( !isResult ) //请求参数
	{
		if ( !AddData(m_count) ) return false;//产生id数量
		return true; 
	}
	//回应参数
	if ( ResultCode::success != m_code ) return true;
	if ( !AddData(m_count) ) return false;//产生id数量
	if ( !AddData(m_id) ) return false;

	return true;
}

bool MsgNewId::Parse()
{
	if (!Buffer::Parse()) return false;

	if ( !IsResult() ) //请求参数
	{
		if ( !GetData(m_count) ) return false;//产生id数量
		return true; 
	}

	// 回应参数
	if ( ResultCode::success != m_code ) return true;
	if ( !GetData(m_count) ) return false;//产生id数量
	if ( !GetData(m_id) ) return false;

	return true;

}

}