#include "RoomInfo.h"


namespace msg
{

RoomInfo::RoomInfo()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


RoomInfo::~RoomInfo()
{
}

bool RoomInfo::Build( bool isResult )
{
	SetId(MsgId::roomInfo, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool RoomInfo::Parse()
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
