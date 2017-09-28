#include "UseItem.h"


namespace msg
{

UseItem::UseItem()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::Room);
}


UseItem::~UseItem()
{
}

bool UseItem::Build( bool isResult )
{
	SetId(MsgId::useItem, isResult);
	if ( !isResult )
	{

	}
	else
	{

	}

	return true;
}

bool UseItem::Parse()
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
