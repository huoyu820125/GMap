#include "MsgSetPoints.h"
#include "Serialize.h"

namespace msg
{
MsgSetPoints::MsgSetPoints()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}


MsgSetPoints::~MsgSetPoints()
{
}


bool MsgSetPoints::Build( bool isResult )
{
	SetId(MsgId::setPoints, isResult);
	if (!FillTransmitParam()) return false;//Tcp服务填写参数

	//请求参数
	if ( !isResult )
	{
		int count = m_points.size();
		m_isEnd = m_startPos < count?false:true;
		if ( !AddData(m_isEnd) ) return false;//顶点列表结束
		if ( !AddData(m_isCreate) ) return false;//是创建操作
		if ( m_isEnd ) return true;

		//顶点列表
		int i = 0;
		int size = 0;
		for ( i = m_startPos; i < count; i++ )
		{
			std::vector<std::string> filter;
			size = m_size;
			if ( !Serialize::AddPoint(*this, m_points[i], true, filter) )
			{
				m_startPos = i;//下次开始打包位置
				m_size = size;//确保截止长度都是完整有效数据的，避免发送时将半打包的数据发送出去
				return true;
			}
		}
		m_startPos = i;//下次开始打包位置

		return true;
	}

	//回应参数
	if ( ResultCode::success != m_code ) return true;

	return true;
}

bool MsgSetPoints::Parse()
{
	if (!Buffer::Parse()) return false;

	// 请求参数
	if ( !IsResult() )
	{
		m_points.clear();
		if ( !GetData(m_isEnd) ) return false;//顶点列表结束
		if ( !GetData(m_isCreate) ) return false;//是创建操作
		if ( m_isEnd ) return true;

		//顶点列表
		std::vector<std::string> filter;
		Grid::Point point;
		if ( 0 >= m_size ) return false;//列表不能为null

		for ( ; m_pos < m_size ; )
		{
			if ( !Serialize::GetPoint(*this, point) )	return false;
			m_points.push_back(point);
		}

		return true;
	}

	// 回应参数
	if ( ResultCode::success != m_code ) return true;

	return true;
}

}
