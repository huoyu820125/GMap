#include "File.h"
#include <cstdio>
#ifdef WIN32
#include <io.h>
#include <direct.h>
#define FSeek64	_fseeki64
#define FTtell64 _ftelli64
#else
#include   <unistd.h>                     //chdir() 
#include   <sys/stat.h>                 //mkdir() 
#include   <sys/types.h>               //mkdir() 
#include   <dirent.h>					//closedir()

#define FSeek64	fseek
#define FTtell64 ftell
#endif


namespace mdk
{
bool File::CreateDir(const std::string &path)
{
	int startPos = 1;
	int pos = path.find( '/', startPos );
	std::string dir;
	while ( true )
	{
		if ( -1 == pos ) dir = path;
		else dir.assign( path, 0, pos );
		if ( -1 == access( dir.c_str(), 0 ) )
		{
#ifdef WIN32
			if( 0 != mkdir(dir.c_str()) ) return false;
#else
			umask(0);
			if( 0 != mkdir(dir.c_str(), 0777) ) return false;
			umask(0);
			chmod(dir.c_str(),0777);
#endif
		}
		if ( -1 == pos ) break;
		startPos = pos + 1;
		pos = path.find( '/', startPos );
	}

	return true;
}

bool File::Exist(const std::string &name)
{
	if ( 0 == access( name.c_str(), 0 ) ) 
	{
#ifndef WIN32
		umask(0);
		chmod(name.c_str(),0777);
#endif
		return true;
	}

	return false;
}

bool File::DelFile(const std::string &dir, const std::string &name)
{
	std::string filename = dir + "/" + name;
	if ( 0 == remove(filename.c_str()) ) return true;
	return false;
}

File::File()
{
	m_fp = NULL;
}

File::File(const std::string &dir, const std::string &name)
{
	m_dir = dir;
	m_name = name;
	m_fp = NULL;
}

File::~File()
{
	Close();
}

void File::SetFile(const std::string &dir, const std::string &name)
{
	m_dir = dir;
	m_name = name;
}

std::string File::GetFileName()
{
	return m_dir + "/" + m_name;
}

bool File::Exist()
{
	std::string filename = m_dir + "/" + m_name;
	return File::Exist(filename);
}

bool File::IsWriteAble()
{
	if ( !Exist() ) return false;
	Close();
	File::Error ret = Open(File::add, File::assii);
	if ( File::success != ret ) return false;
	Close();

	return true;
}

bool File::IsReadAble()
{
	if ( !Exist() ) return false;
	Close();
	File::Error ret = Open(File::read, File::assii);
	if ( File::success != ret ) return false;
	Close();

	return true;
}

File::Error File::Open(const std::string &dir, const std::string &name, File::Action act, File::DataFromat fromat)
{
	if ( NULL != m_fp ) return File::opened;

	m_dir = dir;
	m_name = name;
	return Open(act, fromat);
}

File::Error File::Open(File::Action act, File::DataFromat fromat)
{
	if ( NULL != m_fp ) return File::opened;
	if ( !File::Exist(m_dir) ) 
	{
		if ( !CreateDir(m_dir) ) return File::dirNoExist;
	}
	std::string filename = m_dir + "/" + m_name;
	std::string mode;
	if ( File::read == act ) mode = "r";
	else if ( File::write == act ) mode = "w";
	else if ( File::add == act ) mode = "a";
	else if ( File::readWrite == act ) mode = "r+";

	if ( File::assii == fromat ) mode = mode + "b";

	m_fp = fopen(filename.c_str(), mode.c_str());
	if ( NULL != m_fp ) 
	{
		m_act = act;
		m_fromat = fromat;
		ToTail();
		ToHeader();
	}
	if ( NULL != m_fp ) return File::success;
	if ( !File::Exist(filename) ) return File::fileNoExist;

	return File::tooManyOpen;
}

bool File::Close()
{
	if ( NULL == m_fp ) return true;
	fclose(m_fp);
	m_fp = NULL;

	return true;
}

int64 File::Size()
{
	if ( NULL == m_fp ) return 0;

	return m_size;
}

File::Error File::ToHeader()
{
	if ( NULL == m_fp ) return File::unopen;

	if ( 0 == FSeek64(m_fp, 0, SEEK_SET) ) 
	{
		m_pos = 0;
		return File::success;
	}

	return File::faild;
}

File::Error File::ToTail()
{
	if ( NULL == m_fp ) return File::unopen;
	
	if ( 0 == FSeek64(m_fp, 0, SEEK_END) ) 
	{
		m_size = FTtell64(m_fp);
		m_pos = m_size;
		return File::success;
	}

	return File::faild;
}

File::Error File::Move(int64 pos)
{
	if ( NULL == m_fp ) return File::unopen;
	if ( File::add == m_act ) return refuse;
	if ( 0 == pos ) return File::success;
	if ( m_pos + pos > m_size ) return File::outPos;
	if ( m_pos + pos < 0 ) return File::outPos;

	int n = FSeek64(m_fp, pos, SEEK_CUR);
	m_pos += pos;

	return File::success;
}

bool File::IsEnd()
{
	if ( NULL == m_fp ) return true;

	return m_pos == m_size;
}

File::Error File::Read(void *content, int count)
{
	if ( NULL == m_fp ) return File::unopen;
	if ( 0 >= count ) return File::errorParam;
	if ( File::write == m_act || File::add == m_act ) return File::refuse;

	int ret = fread(content, sizeof(char), count, m_fp);
	if ( count > ret ) return File::faild;
	m_pos += ret;
	if ( ret < count ) return File::unfinished;

	return File::success;
}

File::Error File::Read(std::string &val, int limitedLen)
{
	if ( limitedLen > 1024 ) return File::errorParam;

	short len;
	Error ret = Read(&len, sizeof(short));
	if ( mdk::File::success != ret ) return ret;
	if ( 0 > len || limitedLen < len ) return File::faild;
	char buf[1025];
	if ( 0 < len )
	{
		ret = Read(buf, len);
		if ( mdk::File::success != ret ) return ret;
	}
	buf[len] = 0;
	val = std::string(buf);
	return File::success;
}

File::Error File::Write(void *content, int count)
{
	if ( NULL == m_fp ) return File::unopen;
	if ( 0 >= count ) return File::errorParam;
	if ( File::read == m_act ) return File::refuse;
	int ret = fwrite(content, sizeof(char), count, m_fp);
	if ( count > ret ) return File::faild;
	m_pos += count;

	return File::success;
}

File::Error File::Write(std::string &val, int limitedLen)
{
	if ( limitedLen > 1024 ) return File::errorParam;

	short len = val.size();
	if ( 0 > len || limitedLen < len ) return File::faild;
	Error ret = Write(&len, sizeof(short));
	if ( mdk::File::success != ret ) return ret;
	if ( 0 == len ) return File::success;

	ret = Write((char*)val.c_str(), len);
	if ( mdk::File::success != ret ) return ret;

	return File::success;
}

}
