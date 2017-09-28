#ifndef CALLRESULT_H
#define CALLRESULT_H

#include <string>

typedef struct CallResult
{
	bool isSuccess;//成功
	int	code;//错误码
	std::string reason;//原因
}CallResult;

namespace Call
{
	CallResult Error(int code, const std::string &reason);
	CallResult Success();
}
#endif //CALLRESULT_H
