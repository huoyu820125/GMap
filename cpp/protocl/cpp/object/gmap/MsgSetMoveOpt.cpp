#include "MsgSetMoveOpt.h"
#include "Serialize.h"

namespace msg
{

MsgSetMoveOpt::MsgSetMoveOpt()
{
	BindSetting(m_buffer, MAX_MSG_SIZE, BYTE_ORDER, Moudle::gmap);
}


MsgSetMoveOpt::~MsgSetMoveOpt()
{
}

bool MsgSetMoveOpt::Build( bool isResult )
{
	SetId(MsgId::setMoveOpt, isResult);
	if (!FillTransmitParam()) return false;  // Tcp������д����

	if ( isResult ) return false;//��Ӧ���л�Ӧ

	//�������

	//�ƶ�·��
	unsigned char lineCount = m_selectLines.size();
	if ( Grid::Limit::maxMoveCount < lineCount ) return false;
	if ( !AddData(lineCount) ) return false;
	int j = 0;
	int i = 0;
	unsigned char count;
	for ( j = 0; j < lineCount; j++ )
	{
		Grid::SELECT_LINE &line = m_selectLines[j];
		//�ƶ������ǳ���/���
		if ( !AddData(line.moveOut) ) return false;
		count = line.selectField.size();
		if ( Grid::Limit::maxFilter < count || 0 == count ) return false;
		//��moveOutָ���ķ���������·���ƶ���moveAllLine = trueʱ��selectField��Ч
		if ( !AddData(line.moveAllLine) ) return false;
		if ( !line.moveAllLine )
		{
			if ( !AddData(count) ) return false;
			for ( i = 0; i < count; i++ )
			{
				if ( !AddData(line.selectField[i].cmpMode) ) return false;
				if ( !Serialize::AddField(*this, line.selectField[i].fieldName.c_str(), line.selectField[i].fieldName.size(), line.selectField[i].field)) return false;
			}
		}
	}

	//��������
	count = m_pointFilter.size();
	if ( Grid::Limit::maxFilter < count ) return false;
	if ( !AddData(count) ) return false;
	for ( i = 0; i < count; i++ )
	{
		if ( !AddData(m_pointFilter[i].cmpMode) ) return false;
		if ( !Serialize::AddField(*this, m_pointFilter[i].fieldName.c_str(), m_pointFilter[i].fieldName.size(), m_pointFilter[i].field)) return false;
	}

	if ( !AddData(m_selectAll) ) return false;//ѡȡ�����ֶ�=trueʱm_getFields��Ч

	//ѡ���ֶ�
	count = m_getFields.size();//2048+2
	if ( Grid::Limit::maxFieldCount < count ) return false;
	if ( !AddData(count) ) return false;
	for ( i = 0; i < count; i++ )
	{
		if ( m_getFields[i].size() > Grid::Limit::maxFieldNameSize ) return false;
		if ( !AddData(m_getFields[i]) ) return false;
	}

	if ( !AddData((char)m_act) ) return false;

	return true;
}

bool MsgSetMoveOpt::Parse()
{
	if (!Buffer::Parse()) return false;

	if ( IsResult() ) return false;//�����ڻ�Ӧ����

	// �������
	//�ƶ�·��
	m_selectLines.clear();
	unsigned char lineCount;
	if ( !GetData(lineCount) ) return false;
	if ( 0 > lineCount || lineCount > 10 ) return false;

	int j = 0;
	int i = 0;
	unsigned char count;
	Grid::FILTER filter;
	char fieldName[Grid::Limit::maxFieldNameSize];
	int fieldNameSize;
	for ( j = 0; j < lineCount; j++ )
	{
		//�ƶ������ǳ���/���
		Grid::SELECT_LINE line;
		if ( !GetData(line.moveOut) ) return false;
		//��moveOutָ���ķ���������·���ƶ���moveAllLine = trueʱ��selectField��Ч
		if ( !GetData(line.moveAllLine) ) return false;
		if ( !line.moveAllLine )
		{
			if ( !GetData(count) ) return false;
			if ( 0 > count || Grid::Limit::maxFilter < count ) return false;
			for ( i = 0; i < count; i++ )
			{
				if ( !GetData(filter.cmpMode) ) return false;
				if ( !Serialize::GetField(*this, fieldName, fieldNameSize, filter.field)) return false;
				filter.fieldName = std::string(fieldName, fieldNameSize);
				line.selectField.push_back(filter);
			}
		}
		m_selectLines.push_back(line);
	}

	//��������
	if ( !GetData(count) ) return false;
	if ( 0 > count || Grid::Limit::maxFilter < count ) return false;
	m_pointFilter.clear();
	for ( i = 0; i < count; i++ )
	{
		if ( !GetData(filter.cmpMode) ) return false;
		if ( !Serialize::GetField(*this, fieldName, fieldNameSize, filter.field)) return false;
		filter.fieldName = std::string(fieldName, fieldNameSize);
		m_pointFilter.push_back(filter);
	}

	if ( !GetData(m_selectAll) ) return false;//ѡȡ�����ֶ�=trueʱm_getFields��Ч

	//ѡ���ֶ�
	if ( !GetData(count) ) return false;
	if ( Grid::Limit::maxFieldCount < count ) return false;
	std::string name;
	m_getFields.clear();
	for ( i = 0; i < count; i++ )
	{
		if ( !GetData(name) ) return false;
		if ( name.size() > Grid::Limit::maxFieldNameSize ) return false;
		m_getFields.push_back(name);
	}

	char valc;
	if ( !GetData(valc) ) return false;
	m_act = (Grid::Action)valc;

	return true;
}

}