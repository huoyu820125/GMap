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
	if (!FillTransmitParam()) return false;  // Tcp������д����

	if ( isResult ) return true; //��Ӧ��������

	//�������
	if ( !AddData(m_pointId) ) return false;

	return true;
}

bool MsgDelPoint::Parse()
{
	if (!Buffer::Parse()) return false;

	if ( IsResult() ) return true; // ��Ӧ��������

	// �������
	if ( !GetData(m_pointId) ) return false;

	return true;
}
}