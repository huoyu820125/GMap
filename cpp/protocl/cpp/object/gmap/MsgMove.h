#ifndef MSG_MOVEN_H
#define MSG_MOVEN_H

#include "../../Buffer.h"
#include <vector>

namespace msg
{

//���𶥵�/�߿�ʼ�ƶ�N�Σ���ִ��Ԥ�����Ϊ
class MsgMove : public msg::Buffer
{
public:
	//��������
	int64						m_searchId;//����Ψһid
	bool						m_fromPoint;//�Ӷ��㿪ʼ�ƶ�
	bool						m_filterStartPoint;//�������
	std::vector<int64>			m_ids;//���id�б�
	bool						m_moveStart;//��ʼ�ƶ�
	int							m_startPos;//id�б�����ʼλ��
	//��Ӧ����

public:
	MsgMove();
	virtual ~MsgMove();
	 
 	bool Build( bool isResult = false );
	bool Parse();
};

}
#endif //MSG_MOVEN_H