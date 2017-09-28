#include "MsgMoveMids.h"
#include "Serialize.h"

namespace msg
{

MsgMoveMids::MsgMoveMids()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}


MsgMoveMids::~MsgMoveMids()
{
}

bool MsgMoveMids::Build( char lineIndex, bool isLine, std::vector<int64> &ids, int &startPos )
{
	SetId(MsgId::moveMids, true);
	if (!FillTransmitParam()) return false;  // Tcp������д����

	if ( !AddData(lineIndex) ) return false;
	if ( !AddData(isLine) ) return false;

	int size = m_size;
	int count = ids.size();
	for ( ; startPos < count; startPos++ )
	{
		if ( !AddData(ids[startPos]) )
		{
			m_size = size;//ȷ����ֹ���ȶ���������Ч���ݵģ����ⷢ��ʱ�����������ݷ��ͳ�ȥ
			return true;
		}
		size = m_size;//��¼��Ч���ݳ���
	}

	return true;
}

bool MsgMoveMids::Parse()
{
	if (!Buffer::Parse()) return false;
	if ( !IsResult() ) return false;//������������

	if ( !GetData(m_lineIndex) ) return false;
	if ( !GetData(m_isLine) ) return false;

	if ( m_pos >= m_size ) return false;

	int64 id;
	m_ids.clear();
	for ( ; m_pos < m_size ; )
	{
		if ( !GetData(id) ) return false;
		m_ids.push_back(id);
	}


	return true;
}

}