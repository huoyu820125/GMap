#include "MsgGetLine.h"
#include "Serialize.h"

namespace msg
{

MsgGetLine::MsgGetLine()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}


MsgGetLine::~MsgGetLine()
{
}

bool MsgGetLine::Build( bool isResult )
{
	SetId(MsgId::getLine, isResult);
	if (!FillTransmitParam()) return false;  // Tcp������д����

	//�������
	if ( !isResult )
	{
		if ( !AddData(m_lineId) ) return false;
		return true;
	}

	//��Ӧ����
	if ( ResultCode::success != m_code ) return true;
	if ( !Serialize::AddLine(*this, m_line, true, m_getFields) ) return false;

	return true;
}

bool MsgGetLine::Parse()
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
	if ( !Serialize::GetLine(*this, m_line) ) return false;

	return true;

}

}