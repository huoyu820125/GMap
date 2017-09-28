// Console.h: interface for the Console class.
//
//////////////////////////////////////////////////////////////////////

#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <vector>
#include "mdk/include/mdk/Thread.h"

namespace mdk
{

/*
 *	����̨��
 *	���ܼ������룬ִ��ָ��
 *	ʹ��˵��
 *	1.����Console�����
 *	2.ʵ�ֽ���ָ��Ļص�����������ԭ��char* OnCommand(std::vector<std::string> *cmd)������ָ�������Ϣ��������ӡ�ڿ���̨�ϣ�����Ҫ��ӡ�κ���Ϣ���ɷ���NULL
 *	3.����Start��������̨������ص�����
 */
class Console
{
public:
	/*
		prompt ��������ʾ��
		cmdMaxSize	ָ����󳤶�
	*/
	Console(char *prompt, int cmdMaxSize);
	virtual ~Console();

	/*
		��ʼ����̨
		method	���Ա��ָ��������
		pObj	method���������
		fun		ȫ�ֵ�ָ��������

		ָ������������ԭ��
		char* OnCommand(std::vector<std::string> *cmd);
		cmdָ��+�����б���int main(int argc, char **argv)��argv����˳����ͬ
		OnCommand����ָ�������Ϣ��������ӡ�ڿ���̨�ϣ�����Ҫ��ӡ�κ���Ϣ���ɷ���NULL
	*/
	bool Start( MethodPointer method, void *pObj );
	bool Start( FuntionPointer fun );
	void WaitStop();//�ȴ�����ֹ̨ͣ
	void Stop();
	void SetPrompt( char *prompt );//������ʾ�����������κ�ʱ����ã�Ϊ���޸���ʾ��
private:
	void* RemoteCall InputThread(void*);//ָ������/ִ���߳�
	char* ExecuteCommand(char *cmd);//ִ��ָ��
	void PrintReulst( const char* info );//��ӡ��������Ϣ
		
private:
	bool m_bRun;
	std::string m_prompt;//��������ʾ��
	Thread m_inputThread;//�����߳�
	int m_cmdMaxSize;//ָ����󳤶�
	std::vector<std::string> m_cmd;
	std::vector<std::string> m_waitPrint;
	
	bool m_isMemberMethod;
	MethodPointer m_method;
	void *m_pObj;
	FuntionPointer m_fun;
};

}

#endif // CONSOLE_H
