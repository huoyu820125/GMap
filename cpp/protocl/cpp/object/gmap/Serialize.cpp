#include "Serialize.h"
#include <cstring>

Serialize::Serialize()
{
}

Serialize::~Serialize()
{
}

bool Serialize::AddField(net::Message &msg, const char *name, int nameSize, Grid::FIELD &field )
{
	if ( Grid::Limit::maxFieldNameSize < nameSize || 0 == nameSize ) return false;

	if (!msg.AddData(name, nameSize)) return false;    //属性名(不带\0)
	if (!msg.AddData((char)field.type)) return false;    //属性类型
	if (!msg.AddData(field.value)) return false;    //int属性值
	if ( Grid::str != field.type ) return true;

	field.size = strlen(field.data);
	if ( Grid::Limit::maxFieldSize < field.size || 0 == field.size ) return false;
	field.size++;
	if (!msg.AddData(field.data, field.size)) return false;    //字符串(带\0)属性值

	return true;
}

bool Serialize::GetField(net::Message &msg, char *name, int &nameSize, Grid::FIELD &field )
{
	if ( !msg.GetData(name, nameSize) ) return false;    //属性名(不带\0)
	if ( Grid::Limit::maxFieldNameSize < nameSize ) return false;

	field.size = 0;
	char type;
	if (!msg.GetData(type)) return false;    //属性类型
	field.type = (Grid::DataType)type;
	if ( !msg.GetData(field.value) ) return false;    //属性int值
	if ( Grid::str != field.type ) return true;

	if ( !msg.GetData(field.data, field.size) ) return false;//字符串(带\0)属性值
	if ( Grid::Limit::maxFieldSize < field.size - 1 ) return false;

	return true;
}

bool Serialize::AddFields(net::Message &msg, mdk::Map &fields, bool selectAll, std::vector<std::string> &selectFields )
{
	unsigned char count = 0;
 	mdk::Map::iterator it = fields.begin();
	bool isGet = false;
	int i = 0;
	for ( ; !it.isEnd(); it++ )
	{
		//选择要传输的字段
		isGet = selectAll?true:false;
		for ( i = 0; !isGet && i < selectFields.size(); i++ )
		{
			if ( selectFields[i].size() != it.key()->size ) continue;
			if ( 0 == strncmp(selectFields[i].c_str(), it.key()->data, selectFields[i].size() ) )
			{
				isGet = true;
			}
		}
		if ( !isGet ) continue;
		count++;
	}
	if ( Grid::Limit::maxFieldCount < count ) return false;
	if (!msg.AddData(count)) return false;    //属性数量
	if ( 0 == count ) return true;

	Grid::FIELD *pField;
	for ( it = fields.begin(); !it.isEnd(); it++ )
	{
		//选择要传输的字段
		isGet = selectAll?true:false;
		for ( i = 0; !isGet && i < selectFields.size(); i++ )
		{
			if ( selectFields[i].size() != it.key()->size ) continue;
			if ( 0 == strncmp(selectFields[i].c_str(), it.key()->data, selectFields[i].size() ) )
			{
				isGet = true;
			}
		}
		if ( !isGet ) continue;

		pField = (Grid::FIELD*)it.data();
		if ( !AddField(msg, it.key()->data, it.key()->size, *pField) ) return false;
	}

	return true;
}

bool Serialize::GetFields(net::Message &msg, mdk::Map &fields )
{
	unsigned char count;
	if (!msg.GetData(count)) return false;    //属性数量
	if ( Grid::Limit::maxFieldCount < count ) return false;
	int i = 0;
	char name[Grid::Limit::maxFieldNameSize];
	int nameSize;
	Grid::FIELD *pField;
	for ( ; i < count; i++ )
	{
		pField = new Grid::FIELD;
		if ( !GetField(msg, name, nameSize, *pField) ) return false;
		fields.insert(name, nameSize, pField);
	}

	return true;
}

bool Serialize::AddPoint(net::Message &msg, Grid::Point &point, bool selectAll, std::vector<std::string> &selectFields)
{
	if (!msg.AddData(point.id)) return false;    //顶点id
	return AddFields(msg, point.data, selectAll, selectFields);//数据
}

bool Serialize::GetPoint(net::Message &msg, Grid::Point &point)
{
	if ( 0 > point.out.m_count ) return false;
	if ( 0 > point.in.m_count ) return false;
	point.data.clear();
	if (!msg.GetData(point.id)) return false;    //顶点id
	if ( 0 >= point.id ) return false;
	if (!GetFields(msg, point.data)) return false;//属性

	return true;
}

bool Serialize::AddLine(net::Message &msg, Grid::Line &line, bool selectAll, std::vector<std::string> &selectFields)
{
	if (!msg.AddData(line.id)) return false;    //边id
	if (!msg.AddData(line.startId)) return false;    //起点id
	if (!msg.AddData(line.endId)) return false;    //终点id
	return AddFields(msg, line.data, selectAll, selectFields);
}

bool Serialize::GetLine(net::Message &msg, Grid::Line &line)
{
	line.data.clear();
	if (!msg.GetData(line.id)) return false;    //顶点id
	if (!msg.GetData(line.startId)) return false;    //起点id
	if (!msg.GetData(line.endId)) return false;    //终点id
	if (!GetFields(msg, line.data)) return false; //属性

	return true;
}
