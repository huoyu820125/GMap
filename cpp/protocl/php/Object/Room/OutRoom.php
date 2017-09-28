#include "OutRoom.h"


namespace msg
{

OutRoom::OutRoom()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


OutRoom::~OutRoom()
{
}

bool OutRoom::Build( bool isResult )
{
	SetId(MsgId::outRoom, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool OutRoom::Parse()
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
