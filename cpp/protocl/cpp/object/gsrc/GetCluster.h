#ifndef GET_CLUSTER_H
#define GET_CLUSTER_H

#include "../../Buffer.h"
#include <map>
#include <vector>
#include "common/Common.h"

namespace msg
{

//ȡ��Ⱥ��Ϣ
class GetCluster : public Buffer
{
public:
	//�������
	Moudle::Moudle	m_serviceType; //Ҫȡ�ķ������ͣ�int32   
	int				m_start;      //�ӵ�start������ʼ
	//��Ӧ����

	//map<��·, vector<���>>
	std::map<NetLine::NetLine, std::vector<NODE> > m_cluster;

public:
	GetCluster();
	virtual ~GetCluster();
	bool Build( bool isResult = false);//���챨�ģ�����
	bool Parse();//�������ģ�����

private:
	unsigned char m_buffer[MAX_MSG_SIZE];
};


}
#endif //GET_CLUSTER_H