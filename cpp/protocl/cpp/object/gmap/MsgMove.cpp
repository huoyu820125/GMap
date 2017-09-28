#include "MsgMove.h"

namespace msg
{

MsgMove::MsgMove()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}

MsgMove::~MsgMove()
{
}

bool MsgMove::Build( bool isResult )
{
	SetId(MsgId::move, isResult);
	if (!FillTransmitParam()) return false;  // Tcp服务填写参数

	if ( isResult ) return true;//回应无参数

	//请求
	if ( !AddData(m_searchId) ) return false;//搜索唯一id

	short count = m_ids.size() - m_startPos;
	if ( 0 == count ) return false;

	if ( count > 1000 ) 
	{
		m_moveStart = false;
		count = 1000;
	}
	else m_moveStart = true;
	if ( !AddData(m_moveStart) ) return false;//开始移动

	if ( !AddData(count) ) return false;
	int i = 0;
	for ( ; i < count; i++ )
	{
		if ( !AddData(m_ids[m_startPos]) ) return false;
		m_startPos++;
	}

	if ( !m_moveStart ) return true;

	if ( !AddData(m_fromPoint) ) return false;//从顶点开始移动
	if ( !AddData(m_filterStartPoint) ) return false;//过滤起点

	return true;
}

bool MsgMove::Parse()
{
	if (!Buffer::Parse()) return false;

	if ( IsResult() ) return true;//回应无参数

	//请求
	if ( !GetData(m_searchId) ) return false;//搜索唯一id
	if ( !GetData(m_moveStart) ) return false;//开始移动

	short count;
	if ( !GetData(count) ) return false;
	if ( 0 >= count || count > 1000 ) return false;
	int64 id;
	m_ids.clear();
	int i = 0;
	for ( ; i < count; i++ )
	{
		if ( !GetData(id) ) return false;
		m_ids.push_back(id);
	}
	if ( !m_moveStart ) return true;

	if ( !GetData(m_fromPoint) ) return false;//从顶点开始移动
	if ( !GetData(m_filterStartPoint) ) return false;//过滤起点

	return true;
}

}