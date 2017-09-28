#include "CallResult.h"

namespace Call
{

CallResult Error(int code, const std::string &reason)
{
	CallResult ret;
	ret.isSuccess = false;
	ret.code = code;
	ret.reason = reason;

	return ret;
}

CallResult Success()
{
	CallResult ret;
	ret.isSuccess = true;
	return ret;
}

}

