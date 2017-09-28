#include "LookerPart.h"


namespace msg
{

LookerPart::LookerPart()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


LookerPart::~LookerPart()
{
}

bool LookerPart::Build( bool isResult )
{
	SetId(MsgId::lookerPart, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool LookerPart::Parse()
{
	m_pos = HeaderSize();
	if ( !IsResult() )
	{

	}
	else
	{

	}

	return true;
}

}
