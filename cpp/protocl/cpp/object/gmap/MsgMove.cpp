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
	if (!FillTransmitParam()) return false;  // Tcp������д����

	if ( isResult ) return true;//��Ӧ�޲���

	//����
	if ( !AddData(m_searchId) ) return false;//����Ψһid

	short count = m_ids.size() - m_startPos;
	if ( 0 == count ) return false;

	if ( count > 1000 ) 
	{
		m_moveStart = false;
		count = 1000;
	}
	else m_moveStart = true;
	if ( !AddData(m_moveStart) ) return false;//��ʼ�ƶ�

	if ( !AddData(count) ) return false;
	int i = 0;
	for ( ; i < count; i++ )
	{
		if ( !AddData(m_ids[m_startPos]) ) return false;
		m_startPos++;
	}

	if ( !m_moveStart ) return true;

	if ( !AddData(m_fromPoint) ) return false;//�Ӷ��㿪ʼ�ƶ�
	if ( !AddData(m_filterStartPoint) ) return false;//�������

	return true;
}

bool MsgMove::Parse()
{
	if (!Buffer::Parse()) return false;

	if ( IsResult() ) return true;//��Ӧ�޲���

	//����
	if ( !GetData(m_searchId) ) return false;//����Ψһid
	if ( !GetData(m_moveStart) ) return false;//��ʼ�ƶ�

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

	if ( !GetData(m_fromPoint) ) return false;//�Ӷ��㿪ʼ�ƶ�
	if ( !GetData(m_filterStartPoint) ) return false;//�������

	return true;
}

}