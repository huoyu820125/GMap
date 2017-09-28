#include "TickOutRoom.h"


namespace msg
{

TickOutRoom::TickOutRoom()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


TickOutRoom::~TickOutRoom()
{
}

bool TickOutRoom::Build( bool isResult )
{
	SetId(MsgId::tickOutRoom, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool TickOutRoom::Parse()
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
