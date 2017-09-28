#pragma once

#ifdef WIN32
typedef __int64				int64;
typedef unsigned __int64	uint64;
#else
#include <sys/types.h>
typedef int64_t				int64;
typedef u_int64_t			uint64;
#endif
#include <string>

namespace net
{
	//字节序
	enum ByteOrder
	{
		netOrder = 0,
		bigOrder = 1,
		smallOrder = 2,
		local = 3,
	};

/**
* 流类
*/
class Stream
{
public:
	Stream();
	virtual ~Stream();

public:
	void Bind(unsigned char *buffer, unsigned int space);//绑定一个缓冲区，在此之上做转换
	void SetByteOrder( ByteOrder order );//设置字节序
	operator unsigned char*();
	operator char*();
	operator void*();
	int Size();//实际长度，非法值，小于0
//////////////////////////////////////////////////////////////////////////
//添加参数,成功返回true，失败返回false
	/**
	 * 增加1个参数到消息中,char、unsigned char、int等各种类型的重载
	 * 成功返回true，失败返回false
	 * ※直接传递常量，将自动匹配到4~nbyte中最小的类型
	 * 比如AddData( 1 )将认为是插入一个int，占4byte，如果希望只占1byte，请使用AddData((char)1)
	 * 比如AddData( 0xffffffff )将认为是插入一个unsigned int，占4byte
	 * 比如AddData( 0xffffffff ff)将认为是插入一个int64，占8byte
	*/
	bool AddData(bool value);
	bool AddData(char value);
	inline bool AddData( unsigned char value )
	{
		return AddData( (char)value );
	}
	bool AddData(short value);
	inline bool AddData( unsigned short value )
	{
		return AddData( (short)value );
	}
	bool AddData(int value);
	inline bool AddData( unsigned int value )
	{
		return AddData( (int)value );
	}
	bool AddData(int64 value);
	inline bool AddData( uint64 value )
	{
		return AddData( (int64)value );
	}
	bool AddData( const void* stream, int size );
	bool AddData( const std::string &stream );
	bool AddData(float value);
	bool AddData(double value);
	
//////////////////////////////////////////////////////////////////////////
//取得参数，成功返回true，失败返回false
	/**
	* 从流中取得一个数据char、unsigned char、int等各种类型的重载
	* 成功返回true，失败返回false
	*/
	bool GetData(bool &value);
	bool GetData(char &value);
	bool GetData(unsigned char &value);
	bool GetData(short &value);
	bool GetData(unsigned short &value);
	bool GetData(int &value);
	bool GetData(unsigned int &value);
	bool GetData(int64 &value);
	bool GetData(uint64 &value);
	/**
	 * 从流中取得变长对象
	 * 
	 * 
	 * stream保存对象的缓冲
	 * size输入缓冲容量，返回实际读出大小
	 * 
	 */
	bool GetData(void* stream, int &size);
	bool GetData(std::string &str);
	/**
	 * 为了免去Copy，直接从流中取得变长对象的指针地址
	 * 成功返回对象指针，失败返回NULL
	 * uSize返回地址大小
	*/
	void* GetPointer( int &size );
	bool GetData(float &value);
	bool GetData(double &value);

protected:
	//将整型存入流，size表示整型长度char short int int64
	void I2Stream( unsigned char *buf, uint64 value, int size );
	//从流中取出整型参数，size表示整型长度char short int int64
	uint64 Stream2I( unsigned char *buf, int size );

	void StartAdd();
	void StartGet();

protected:
	unsigned char*	m_buffer;		//绑定的缓冲区
	unsigned int	m_space;		//缓冲大小
	int				m_size;			//流实际长度
	int				m_pos;			//游标位置
	ByteOrder		m_byteOrder;	//报文使用的字节顺
	bool			m_building;		//报文构建中
};

}
