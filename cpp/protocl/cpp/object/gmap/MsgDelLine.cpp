#include "MsgDelLine.h"

namespace msg
{

MsgDelLine::MsgDelLine()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}


MsgDelLine::~MsgDelLine()
{
}

bool MsgDelLine::Build( bool isResult )
{
	SetId(MsgId::delLine, isResult);
	if (!FillTransmitParam()) return false;  // Tcp������д����

	//�������
	if ( !isResult ) 
	{
		if ( !AddData(m_lineId) ) return false;
		return true;
	}

	//��Ӧ����
	if ( ResultCode::success != m_code ) return true;
	if ( !AddData(m_startId) ) return false;
	if ( !AddData(m_endId) ) return false;

	return true;
}

bool MsgDelLine::Parse()
{
	if (!Buffer::Parse()) return false;

	// �������
	if ( !IsResult() )
	{
		if ( !GetData(m_lineId) ) return false;
		return true;
	}

	// ��Ӧ����
	if ( ResultCode::success != m_code ) return true;
	if ( !GetData(m_startId) ) return false;
	if ( !GetData(m_endId) ) return false;

	return true;

}

}