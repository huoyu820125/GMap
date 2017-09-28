#ifndef OWNLINES_H
#define OWNLINES_H

#include "common/Grid.h"

namespace Grid
{

class Line;

//bigint����id�б�
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
	ELEMENT		*m_list;//����
	int			m_count;//����

private:
	int			m_space;//����
};

}

#endif //#define OWNLINES_H