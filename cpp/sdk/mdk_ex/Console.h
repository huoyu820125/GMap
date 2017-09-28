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
 *	控制台类
 *	接受键盘输入，执行指令
 *	使用说明
 *	1.创建Console类对象
 *	2.实现接受指令的回调方法，函数原形char* OnCommand(std::vector<std::string> *cmd)，返回指令处理结果信息，将被打印在控制台上，不需要打印任何信息，可返回NULL
 *	3.调用Start启动控制台，传入回调函数
 */
class Console
{
public:
	/*
		prompt 命令行提示符
		cmdMaxSize	指令最大长度
	*/
	Console(char *prompt, int cmdMaxSize);
	virtual ~Console();

	/*
		开始控制台
		method	类成员的指令受理方法
		pObj	method所属类对象
		fun		全局的指令受理方法

		指令受理方法申明原形
		char* OnCommand(std::vector<std::string> *cmd);
		cmd指令+参数列表，与int main(int argc, char **argv)的argv参数顺序相同
		OnCommand返回指令处理结果信息，将被打印在控制台上，不需要打印任何信息，可返回NULL
	*/
	bool Start( MethodPointer method, void *pObj );
	bool Start( FuntionPointer fun );
	void WaitStop();//等待控制台停止
	void Stop();
	void SetPrompt( char *prompt );//设置提示符，可以在任何时候调用，为了修改提示符
private:
	void* RemoteCall InputThread(void*);//指令输入/执行线程
	char* ExecuteCommand(char *cmd);//执行指令
	void PrintReulst( const char* info );//打印处理结果信息
		
private:
	bool m_bRun;
	std::string m_prompt;//命令行提示符
	Thread m_inputThread;//输入线程
	int m_cmdMaxSize;//指令最大长度
	std::vector<std::string> m_cmd;
	std::vector<std::string> m_waitPrint;
	
	bool m_isMemberMethod;
	MethodPointer m_method;
	void *m_pObj;
	FuntionPointer m_fun;
};

}

#endif // CONSOLE_H
