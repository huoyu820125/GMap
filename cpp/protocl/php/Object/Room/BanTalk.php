#include "BanTalk.h"


namespace msg
{

BanTalk::BanTalk()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


BanTalk::~BanTalk()
{
}

bool BanTalk::Build( bool isResult )
{
	SetId(MsgId::banTalk, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool BanTalk::Parse()
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
