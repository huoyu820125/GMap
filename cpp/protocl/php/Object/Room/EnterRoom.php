#include "EnterRoom.h"


namespace msg
{

EnterRoom::EnterRoom()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


EnterRoom::~EnterRoom()
{
}

bool EnterRoom::Build( bool isResult )
{
	SetId(MsgId::enterRoom, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool EnterRoom::Parse()
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
