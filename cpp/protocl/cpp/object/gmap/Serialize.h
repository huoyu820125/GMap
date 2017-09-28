#ifndef SERIALIZE_H
#define SERIALIZE_H
#include "../../base/Message.h"
#include "common/struct/Point.h"
#include "common/struct/Line.h"
#include "mdk_ex/container/Map.h"

class Serialize
{
public:
	Serialize();
	virtual ~Serialize();

	static bool AddField(net::Message &msg, const char *name, int nameSize, Grid::FIELD &field );
	static bool GetField(net::Message &msg, char *name, int &nameSize, Grid::FIELD &field );
	static bool AddFields(net::Message &msg, mdk::Map &fields, bool selectAll, std::vector<std::string> &selectFields );
	static bool GetFields(net::Message &msg, mdk::Map &fields );
	static bool AddPoint(net::Message &msg, Grid::Point &point, bool selectAll, std::vector<std::string> &selectFields);
	static bool GetPoint(net::Message &msg, Grid::Point &point);
	static bool AddLine(net::Message &msg, Grid::Line &line, bool selectAll, std::vector<std::string> &selectFields);
	static bool GetLine(net::Message &msg, Grid::Line &line);
};

#endif //SERIALIZE_H