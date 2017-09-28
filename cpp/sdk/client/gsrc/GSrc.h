#ifndef CLUSTERCFGCLI_H
#define CLUSTERCFGCLI_H

#include "protocl/cpp/base/Socket.h"
#include "protocl/cpp/Buffer.h"
#include <string>
#include <map>
#include <cstdlib>
#include <cstdio>

#include "protocl/cpp/object/gsrc/GetCluster.h"
#include "protocl/cpp/object/gsrc/MsgGetClientId.h"
#include "common/CallResult.h"
#include "../Client.h"

class GSrc : public Client
{
public:
	GSrc(void);
	virtual ~GSrc(void);

	//»°∑˛ŒÒ≈‰÷√
	CallResult GetServices(std::map<NetLine::NetLine, std::vector<NODE> > &cluster, Moudle::Moudle svrType);
	CallResult GetClientId(int64 &clientId);
};

#endif //CLUSTERCFGCLI_H