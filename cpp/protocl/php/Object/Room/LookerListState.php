#include "LookerListState.h"


namespace msg
{

LookerListState::LookerListState()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


LookerListState::~LookerListState()
{
}

bool LookerListState::Build( bool isResult )
{
	SetId(MsgId::lookerListState, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool LookerListState::Parse()
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
