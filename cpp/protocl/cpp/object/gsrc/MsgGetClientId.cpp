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
	if (!FillTransmitParam()) return false;  // Tcp������д����

	//�������

	//��Ӧ����
	if (!isResult || ResultCode::success != m_code) return true;
	if ( !AddData(m_clientId) ) return false;//cientΨһid

	return true;
}

bool MsgGetClientId::Parse()
{
	if (!Buffer::Parse()) return false;

	// �������

	// ��Ӧ����
	if (!IsResult() || ResultCode::success != m_code) return true;
	if ( !GetData(m_clientId) ) return false;//cientΨһid

	return true;

}

}