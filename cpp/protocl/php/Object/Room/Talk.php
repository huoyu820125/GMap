#include "Talk.h"


namespace msg
{

Talk::Talk()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


Talk::~Talk()
{
}

bool Talk::Build( bool isResult )
{
	SetId(MsgId::talk, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool Talk::Parse()
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
