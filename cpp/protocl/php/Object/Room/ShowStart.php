#include "ShowStart.h"


namespace msg
{

ShowStart::ShowStart()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


ShowStart::~ShowStart()
{
}

bool ShowStart::Build( bool isResult )
{
	SetId(MsgId::showStart, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool ShowStart::Parse()
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
