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
	if (!FillTransmitParam()) return false;  // Tcp服务填写参数

	int size = m_size;
	for ( ; startPos < count; startPos++ )
	{
		if ( !Serialize::AddPoint(*this, *(points[startPos]), selectAll, getFields) )
		{
			m_size = size;//确保截止长度都是完整有效数据的，避免发送时将半打包的数据发送出去
			return true;
		}
		size = m_size;//记录有效数据长度
	}

	return true;
}

bool MsgPoints::Parse()
{
	if (!Buffer::Parse()) return false;
	if ( !IsResult() ) return false;//不存在请求报文

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
