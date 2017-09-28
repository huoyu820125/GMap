#include "MsgSetPoint.h"
#include "Serialize.h"

namespace msg
{

MsgSetPoint::MsgSetPoint()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}

MsgSetPoint::~MsgSetPoint()
{
}

bool MsgSetPoint::Build( bool isResult )
{
	SetId(MsgId::setPoint, isResult);
	if (!FillTransmitParam()) return false;  // Tcp������д����

 	if ( isResult ) return true; //��Ӧ�޲���

	//�������
	std::vector<std::string> filter;
	if ( !Serialize::AddPoint(*this, m_point, true, filter) )	return false;//����

	return true;
}

bool MsgSetPoint::Parse()
{
	if (!Buffer::Parse()) return false;

	if ( IsResult() ) return true;//��Ӧ�޲���

	// �������
	return Serialize::GetPoint(*this, m_point);//����
}

}