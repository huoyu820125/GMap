#ifndef COMMON_H
#define COMMON_H

#include <string>

namespace NetLine
{
	enum NetLine
	{
		unknow = 0,	//δ֪��·
		CTCC = 1,	//�й�����  chinaTelecom  
		CMCC = 2,	//�й��ƶ�ͨ�� chinamobile
		CUCC = 3,	//�й���ͨͨѶ chinaunicom    
		localNet = 4, //������
	};
}

// ������
typedef struct NODE
{
	int nodeId;
	std::string ip;
	int port;
}NODE;

#endif //COMMON_H
