#ifndef MSG_NEW_ID_H
#define MSG_NEW_ID_H

#include "../../Buffer.h"

namespace msg
{

//����m_count��ΨһId
class MsgNewId : public msg::Buffer
{
public:
	//��������
	int  m_count;//����id����

	//��Ӧ����
	int64 m_id;//Ψһid

public:
	MsgNewId();
	virtual ~MsgNewId();
	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_NEW_ID_H