// MsgHelper.cpp : 定义控制台应用程序的入口点。
//

#include "..\Object\Auth\UserLogin.h"

int main(int argc, char* argv[])
{
	msg::UserLogin out;
	msg::UserLogin in;
	out.m_connectId = 100;//链接Id	Tcp服务填写（回应必填）
	out.m_clientType = msg::UserLogin::flash;//Client类型（回应不填写）
	out.m_accountType = msg::UserLogin::qq;//账号类型，（回应失败不填写）
	out.m_account = "49038554";//账号	账号/手机/qq号码/email（回应失败不填写）
	out.m_pwd = "123456";//密码	md5(明文)（回应不填写）
	out.m_code = 0;
	out.m_reason = "密码错误";
	out.m_userId = 60001;
	out.m_token = "123456789";
	out.Build(true);

	memcpy(in, out, out.Size());
	if ( -1 == in.Size() )
	{
		return 0;
	}
	if ( !in.Parse() )
	{
		return 0;
	}
	return 0;
}

