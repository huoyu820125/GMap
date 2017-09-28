#include "MsgPoints.h"
#include "Serialize.h"

namespace msg
{

MsgPoints::MsgPoints()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}


MsgPoints::~MsgPoints()
{
}

bool MsgPoints::Build(Grid::Point **points, int count, int &startPos, bool selectAll, std::vector<std::string> &getFields)
{
	SetId(MsgId::points, true);
	if (!FillTransmitParam()) return false;  // Tcp������д����

	int size = m_size;
	for ( ; startPos < count; startPos++ )
	{
		if ( !Serialize::AddPoint(*this, *(points[startPos]), selectAll, getFields) )
		{
			m_size = size;//ȷ����ֹ���ȶ���������Ч���ݵģ����ⷢ��ʱ�����������ݷ��ͳ�ȥ
			return true;
		}
		size = m_size;//��¼��Ч���ݳ���
	}

	return true;
}

bool MsgPoints::Parse()
{
	if (!Buffer::Parse()) return false;
	if ( !IsResult() ) return false;//������������

	m_points.clear();

	if ( m_pos >= m_size ) return false;

	for ( ; m_pos < m_size ; )
	{
		Grid::Point point;
		if ( !Serialize::GetPoint(*this, point) ) return false;
		m_points.push_back(point);
	}

	return true;
}

}
