#include "MsgGetClientId.h"

namespace msg
{

MsgGetClientId::MsgGetClientId()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gsrc);
}


MsgGetClientId::~MsgGetClientId()
{
}

bool MsgGetClientId::Build( bool isResult )
{
	SetId(MsgId::getClientId, isResult);
	if (!FillTransmitParam()) return false;  // Tcp服务填写参数

	//请求参数

	//回应参数
	if (!isResult || ResultCode::success != m_code) return true;
	if ( !AddData(m_clientId) ) return false;//cient唯一id

	return true;
}

bool MsgGetClientId::Parse()
{
	if (!Buffer::Parse()) return false;

	// 请求参数

	// 回应参数
	if (!IsResult() || ResultCode::success != m_code) return true;
	if ( !GetData(m_clientId) ) return false;//cient唯一id

	return true;

}

}