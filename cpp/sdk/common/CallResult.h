#ifndef CALLRESULT_H
#define CALLRESULT_H

#include <string>

typedef struct CallResult
{
	bool isSuccess;//�ɹ�
	int	code;//������
	std::string reason;//ԭ��
}CallResult;

namespace Call
{
	CallResult Error(int code, const std::string &reason);
	CallResult Success();
}
#endif //CALLRESULT_H
