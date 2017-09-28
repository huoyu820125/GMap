#ifndef OWNLINES_H
#define OWNLINES_H

#include "common/Grid.h"

namespace Grid
{

class Line;

//bigint类型id列表
class OwnLines
{
public:
	OwnLines(void);
	virtual ~OwnLines(void);

	void Release();

	void Add(bigint id);
	bool Del(bigint id);
	bool DelByIndex(int i);

public:
	typedef struct ELEMENT
	{
		bigint id;
		Grid::Line *pointer;
	}ELEMENT;
	ELEMENT		*m_list;//数组
	int			m_count;//数量

private:
	int			m_space;//容量
};

}

#endif //#define OWNLINES_H