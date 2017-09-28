#include "MsgNewId.h"

namespace msg
{

MsgNewId::MsgNewId()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
	m_count = 1;//����id����
}


MsgNewId::~MsgNewId()
{
}

bool MsgNewId::Build( bool isResult )
{
	SetId(MsgId::newId, isResult);
	if (!FillTransmitParam()) return false;  // Tcp������д����

	if ( !isResult ) //�������
	{
		if ( !AddData(m_count) ) return false;//����id����
		return true; 
	}
	//��Ӧ����
	if ( ResultCode::success != m_code ) return true;
	if ( !AddData(m_count) ) return false;//����id����
	if ( !AddData(m_id) ) return false;

	return true;
}

bool MsgNewId::Parse()
{
	if (!Buffer::Parse()) return false;

	if ( !IsResult() ) //�������
	{
		if ( !GetData(m_count) ) return false;//����id����
		return true; 
	}

	// ��Ӧ����
	if ( ResultCode::success != m_code ) return true;
	if ( !GetData(m_count) ) return false;//����id����
	if ( !GetData(m_id) ) return false;

	return true;

}

}