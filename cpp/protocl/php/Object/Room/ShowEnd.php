#include "ShowEnd.h"


namespace msg
{

ShowEnd::ShowEnd()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


ShowEnd::~ShowEnd()
{
}

bool ShowEnd::Build( bool isResult )
{
	SetId(MsgId::showEnd, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool ShowEnd::Parse()
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
