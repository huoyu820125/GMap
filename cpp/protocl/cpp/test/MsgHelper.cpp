// MsgHelper.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "..\Object\Auth\UserLogin.h"

int main(int argc, char* argv[])
{
	msg::UserLogin out;
	msg::UserLogin in;
	out.m_connectId = 100;//����Id	Tcp������д����Ӧ���
	out.m_clientType = msg::UserLogin::flash;//Client���ͣ���Ӧ����д��
	out.m_accountType = msg::UserLogin::qq;//�˺����ͣ�����Ӧʧ�ܲ���д��
	out.m_account = "49038554";//�˺�	�˺�/�ֻ�/qq����/email����Ӧʧ�ܲ���д��
	out.m_pwd = "123456";//����	md5(����)����Ӧ����д��
	out.m_code = 0;
	out.m_reason = "�������";
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

