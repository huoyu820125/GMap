#include "RoomFrozen.h"


namespace msg
{

RoomFrozen::RoomFrozen()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


RoomFrozen::~RoomFrozen()
{
}

bool RoomFrozen::Build( bool isResult )
{
	SetId(MsgId::roomFrozen, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool RoomFrozen::Parse()
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
