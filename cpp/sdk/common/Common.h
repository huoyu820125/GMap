#ifndef COMMON_H
#define COMMON_H

#include <string>

namespace NetLine
{
	enum NetLine
	{
		unknow = 0,	//未知线路
		CTCC = 1,	//中国电信  chinaTelecom  
		CMCC = 2,	//中国移动通信 chinamobile
		CUCC = 3,	//中国联通通讯 chinaunicom    
		localNet = 4, //局域网
	};
}

// 服务结点
typedef struct NODE
{
	int nodeId;
	std::string ip;
	int port;
}NODE;

#endif //COMMON_H
