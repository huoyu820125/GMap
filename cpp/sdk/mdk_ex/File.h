#ifndef MDK_FILE_H
#define MDK_FILE_H
#include "mdk/include/mdk/FixLengthInt.h"

#include <string>

#ifdef WIN32
#else
#define LARGE_FILE
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64
#endif

namespace mdk
{

//-D_FILE_OFFSET_BITS=64
class File
{
public:
	File();
	File(const std::string &dir, const std::string &name);
	virtual ~File();

	static bool Exist(const std::string &name);
	static bool CreateDir(const std::string &path);
	static bool DelFile(const std::string &dir, const std::string &name);
	enum Error
	{
		success = 0,
		faild = 1,
		opened = 2, 
		unopen = 3,
		refuse = 4,//拒绝操作，一般是打开模式决定了操作是否允许
		outPos = 5,
		errorParam = 6,
		dirNoExist = 7,
		fileNoExist = 8,
		unfinished = 9,
		tooManyOpen = 10,
	};
	enum Action
	{
		read = 0,//打开文件（如果文件不存在则打开失败），读取
		write = 1,//打开文件（如果文件不存在则创建文件），从头部写入
		add = 2,//打开文件（如果文件不存在则打开失败），往文件末尾添加内容
		readWrite = 3,//打开文件后，在执行写入前可读取，一旦执行了写操作后，则不能再读取
	};
	enum DataFromat
	{
		txt = 0,//文本方式IO
		assii = 1,//二进制方式IO
	};
	void SetFile(const std::string &dir, const std::string &name);
	std::string GetFileName();
	bool Exist();
	bool IsWriteAble();
	bool IsReadAble();
	Error Open(const std::string &dir, const std::string &name, Action act, DataFromat fromat);
	Error Open(Action act, DataFromat fromat);
	int64	Size();
	bool Close();
	Error ToHeader();
	Error ToTail();
	Error Move(int64 pos);
	bool IsEnd();
	Error Read(void *content, int count);
	Error Read(std::string &val, int limitedLen);
	Error Write(void *content, int count);
	Error Write(std::string &val, int limitedLen);

private:
	std::string m_dir;
	std::string m_name;
	FILE *m_fp;
	Action m_act;
	DataFromat m_fromat;
	int64 m_pos;
	int64 m_size;
};

}

#endif //MDK_FILE_H