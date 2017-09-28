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
	if (!FillTransmitParam()) return false;//Tcp������д����

	//�������
	if ( !isResult )
	{
		int count = m_points.size();
		m_isEnd = m_startPos < count?false:true;
		if ( !AddData(m_isEnd) ) return false;//�����б����
		if ( !AddData(m_isCreate) ) return false;//�Ǵ�������
		if ( m_isEnd ) return true;

		//�����б�
		int i = 0;
		int size = 0;
		for ( i = m_startPos; i < count; i++ )
		{
			std::vector<std::string> filter;
			size = m_size;
			if ( !Serialize::AddPoint(*this, m_points[i], true, filter) )
			{
				m_startPos = i;//�´ο�ʼ���λ��
				m_size = size;//ȷ����ֹ���ȶ���������Ч���ݵģ����ⷢ��ʱ�����������ݷ��ͳ�ȥ
				return true;
			}
		}
		m_startPos = i;//�´ο�ʼ���λ��

		return true;
	}

	//��Ӧ����
	if ( ResultCode::success != m_code ) return true;

	return true;
}

bool MsgSetPoints::Parse()
{
	if (!Buffer::Parse()) return false;

	// �������
	if ( !IsResult() )
	{
		m_points.clear();
		if ( !GetData(m_isEnd) ) return false;//�����б����
		if ( !GetData(m_isCreate) ) return false;//�Ǵ�������
		if ( m_isEnd ) return true;

		//�����б�
		std::vector<std::string> filter;
		Grid::Point point;
		if ( 0 >= m_size ) return false;//�б���Ϊnull

		for ( ; m_pos < m_size ; )
		{
			if ( !Serialize::GetPoint(*this, point) )	return false;
			m_points.push_back(point);
		}

		return true;
	}

	// ��Ӧ����
	if ( ResultCode::success != m_code ) return true;

	return true;
}

}
