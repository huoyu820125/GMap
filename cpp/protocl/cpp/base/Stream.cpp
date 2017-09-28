#include <cstring>
#include "Stream.h"

#ifdef WIN32
#include <Windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#endif



//将一个整数存储到字节流buf，按照小端字节序(低位在前，高位在后)
#define InSmall( buf, value, size )\
{\
	int Var_move = (size-1) * 8;\
	int Var_del = (size-1) * 8;\
	int Var_index = 0;\
	for ( Var_index = 0; Var_index < size; Var_index++ )\
	{\
		buf[Var_index] = (char)((value << Var_move) >> Var_del);\
		Var_move -= 8;\
	}\
}

//将buf字节流专程整数，按照小端字节序(低位在前，高位在后)
#define OutSmall( value, buf, size )\
{\
	value = 0;\
	int Var_index = 0;\
	value += buf[size - 1];\
	for ( Var_index = size - 2; 0 <= Var_index; Var_index-- )\
	{\
		value <<= 8;\
		value += buf[Var_index];\
	}\
}

//将一个整数存储到字节流buf，按照大端字节序(网络序，高位在前，低位在后)
#define InBig( buf, value, size )\
{\
	int Var_move = (size-1) * 8;\
	int Var_del = (size-1) * 8;\
	int Var_index = size - 1 ;\
	for ( ; Var_index >= 0; Var_index-- )\
	{\
		buf[Var_index] = (char)((value << Var_move) >> Var_del);\
		Var_move -= 8;\
	}\
}

//将buf字节流专程整数，按照大端字节序(网络序，高位在前，低位在后)
#define OutBig( value, buf, size )\
{\
	value = 0;\
	int Var_index = 0;\
	value += buf[Var_index];\
	for ( Var_index = 1; Var_index < size; Var_index++ )\
	{\
		value <<= 8;\
		value += buf[Var_index];\
	}\
}

//将一个整数存储到字节流buf，按照byteOrder字节序(0网络序，1大端，2小端)
#define InStream(buf, value, size, m_byteOrder) \
{\
	if ( net::local == m_byteOrder ) \
	{\
		if ( 1 == size ) (buf)[0] = value;\
		else if ( 2 == size ) \
		{\
			unsigned short Var_v = value;\
			memcpy((buf), &Var_v, size);\
		}\
		else if ( 4 == size ) \
		{\
			unsigned int Var_v = value;\
			memcpy((buf), &Var_v, size);\
		}\
		else memcpy((buf), &value, size);\
	}\
	else if ( net::netOrder == m_byteOrder )\
	{\
		if ( size < 8 ) \
		{\
			int high = value >> 32;\
			int low = value;\
\
			if ( 4 < size )\
			{\
				int netOrder = htonl(high);\
				memcpy((buf), &netOrder, sizeof(int));\
				netOrder = htonl(low);\
				memcpy(&(buf)[sizeof(int)], &netOrder, sizeof(int));\
			}\
			else \
			{\
				int netOrder = htonl(low);\
				netOrder = netOrder >> (4 - size) * 8;\
				memcpy((buf), &netOrder, size);\
			}\
		}\
	}\
	else if ( net::bigOrder == m_byteOrder ) \
	{\
		InBig((buf), value, size);\
	}\
	else \
	{\
		InSmall((buf), value, size);\
	}\
}

//将buf字节流专程整数，按照byteOrder字节序(0网络序，1大端，2小端)
#define OutStream( retVal, buf, size, m_byteOrder )\
{\
	if ( net::local == m_byteOrder )\
	{\
		if ( 1 == size ) retVal = (buf)[0];\
		else if ( 2 == size ) \
		{\
			unsigned short Var_v;\
			memcpy(&Var_v, (buf), size);\
			retVal = Var_v;\
		}\
		else if ( 4 == size ) \
		{\
			unsigned int Var_v;\
			memcpy(&Var_v, (buf), size);\
			retVal = Var_v;\
		}\
		else \
		{\
			uint64 Var_v;\
			memcpy(&Var_v, (buf), size);\
			retVal = Var_v;\
		}\
	}\
	else if ( net::netOrder == m_byteOrder )\
	{\
		if ( size > 8 ) retVal = 0;\
\
		uint64 Var_value = 0;\
		if ( 4 < size )\
		{\
			unsigned int Var_high = ntohl(*((unsigned int*)(buf)) );\
			unsigned int Var_low = ntohl(*((unsigned int*)&(buf)[sizeof(int)]) );\
			Var_value = Var_high;\
			Var_value = Var_value << 32;\
			Var_value += Var_low;\
		}\
		else \
		{\
			char Var_stream[4];\
			int Var_index = 0;\
			for ( Var_index = 0; Var_index < 4 - size; Var_index++ ) Var_stream[Var_index] = 0;\
			int Var_j = 0;\
			for ( ; Var_index < 4; Var_index++ ) Var_stream[Var_index] = (buf)[Var_j++];\
			unsigned int Var_low = ntohl(*((unsigned int*)Var_stream) );\
			Var_value = Var_low;\
		}\
\
		retVal = Var_value;\
	}\
	else if ( net::bigOrder == m_byteOrder ) \
	{\
		OutBig(retVal, (buf), size)\
	}\
	else \
	{\
		OutSmall(retVal, (buf), size)\
	}\
}




//将一个整数存储到字节流buf，按照大端字节序(网络序，高位在前，低位在后)
static void itomemBig( unsigned char *buf, uint64 value, int size )
{
	int move = (size-1) * 8;
	int del = (size-1) * 8;
	int i = size - 1 ;
	for ( ; i >= 0; i-- )
	{
		buf[i] = (char)((value << move) >> del);
		move -= 8;
	}
}

//将buf字节流专程整数，按照大端字节序(网络序，高位在前，低位在后)
static uint64 memtoiBig( unsigned char *buf, int size )
{
	uint64 value = 0;
	int i = 0;
	value += buf[i];
	for ( i = 1; i < size; i++ )
	{
		value <<= 8;
		value += buf[i];
	}
	return value;
}

//将一个整数存储到字节流buf，按照小端字节序(低位在前，高位在后)
static void itomemSmall( unsigned char *buf, uint64 value, int size )
{
	int move = (size-1) * 8;
	int del = (size-1) * 8;
	int i = 0;
	for ( i = 0; i < size; i++ )
	{
		buf[i] = (char)((value << move) >> del);
		move -= 8;
	}
}

//将buf字节流专程整数，按照小端字节序(低位在前，高位在后)
static uint64 memtoiSmall( unsigned char *buf, int size )
{
	uint64 value = 0;
	int i = 0;
	value += buf[size - 1];
	for ( i = size - 2; 0 <= i; i-- )
	{
		value <<= 8;
		value += buf[i];
	}
	return value;
}


namespace net
{

Stream::Stream()
:m_buffer(NULL), m_space(0), m_size(4), m_pos(4), m_byteOrder(bigOrder)
{
	m_building = true;
}

Stream::~Stream()
{
}

void Stream::Bind(unsigned char *pBuffer, unsigned int space)
{
	m_buffer = pBuffer;
	m_space = space;
}

void Stream::SetByteOrder( ByteOrder order )
{
	int i = 0x01020304;
	unsigned char local[sizeof(int)];
	memcpy(local, &i, sizeof(int));
	m_byteOrder = order;
	unsigned char set[sizeof(int)];
// 	I2Stream(set, i, sizeof(int));
	InStream(set, i, sizeof(int), m_byteOrder);
	for ( i = 0; i < sizeof(int); i++ )
	{
		if ( local[i] != set[i] ) return;
	}
	//设置字节序与本机相同，直接使用memcpy性能可提高2倍
 	m_byteOrder = net::local;
}

Stream::operator unsigned char*()
{
	if ( m_building ) 
	{
// 		I2Stream(m_buffer, m_size, sizeof(int));
		InStream(m_buffer, m_size, sizeof(int), m_byteOrder);
	}
	return m_buffer;
}

Stream::operator char*()
{
	if ( m_building ) 
	{
// 		I2Stream(m_buffer, m_size, sizeof(int));
		InStream(m_buffer, m_size, sizeof(int), m_byteOrder);
	}
	return (char*)m_buffer;
}

Stream::operator void*()
{
	if ( m_building ) 
	{
//		I2Stream(m_buffer, m_size, sizeof(int));
		InStream(m_buffer, m_size, sizeof(int), m_byteOrder);
	}
	return m_buffer;
}

int Stream::Size()
{
	return m_size;
}

bool Stream::AddData(bool value)
{
	if ( m_size + sizeof(char) > m_space ) return false;
	if ( !value ) m_buffer[m_size] = 0;
	else m_buffer[m_size] = 1;
	m_size += sizeof(char);

	return true;
}

bool Stream::AddData( char value )
{
	if ( m_size + sizeof(char) > m_space ) return false;
	m_buffer[m_size] = value;
	m_size += sizeof(char);

	return true;
}

bool Stream::AddData( short value )
{
	if ( m_size + sizeof(short) > m_space ) return false;
// 	I2Stream(&m_buffer[m_size], value, sizeof(short));
	InStream(&m_buffer[m_size], value, sizeof(short), m_byteOrder)
	m_size += sizeof(short);
	
	return true;
}

bool Stream::AddData( int value )
{
	if ( m_size + sizeof(int) > m_space ) return false;
//	I2Stream(&m_buffer[m_size], value, sizeof(int));
	InStream(&m_buffer[m_size], value, sizeof(int), m_byteOrder);
	m_size += sizeof(int);

	return true;
}

bool Stream::AddData( int64 value )
{
	if ( m_size + sizeof(int64) > m_space ) return false;
//	I2Stream(&m_buffer[m_size], value, sizeof(int64));
	InStream(&m_buffer[m_size], value, sizeof(int64), m_byteOrder);
	m_size += sizeof(int64);

	return true;
}

bool Stream::AddData( const void* stream, int size )
{
	if ( 0 >= size ) return false;
	if ( !AddData( size ) ) return false;
	if ( m_size + size > m_space ) 
	{
		m_size -= sizeof(int);
		return false;
	}
	memcpy( &m_buffer[m_size], stream, size );
	m_size += size;
	
	return true;
}

bool Stream::AddData( const std::string &stream )
{
	if ( 0 == stream.size() ) return false;
	if ( !AddData( (int)(stream.size() + 1) ) ) return false;
	if ( m_size + stream.size() + 1 > m_space ) 
	{
		m_size -= sizeof(int);
		return false;
	}
	memcpy( &m_buffer[m_size], stream.data(), stream.size() );
	m_size += stream.size();
	m_buffer[m_size] = '\0';
	m_size += 1;

	return true;
}

bool Stream::AddData(float value)
{
	if ( m_size + sizeof(float) > m_space ) return false;
	memcpy( &m_buffer[m_size], &value, sizeof(float) );
	m_size += sizeof(float);

	return true;
}

bool Stream::AddData(double value)
{
	if ( m_size + sizeof(double) > m_space ) return false;
	memcpy( &m_buffer[m_size], &value, sizeof(double) );
	m_size += sizeof(double);

	return true;
}

bool Stream::GetData(bool &value)
{
	if ( m_pos + sizeof(char) > m_size ) return false;
	if ( 0 == m_buffer[m_pos] ) value = false;
	else value = true;
	m_pos += sizeof(char);

	return true;
}

bool Stream::GetData( char &value )
{
	if ( m_pos + sizeof(char) > m_size ) return false;
	value = m_buffer[m_pos];
	m_pos += sizeof(char);

	return true;
}

bool Stream::GetData( unsigned char &value )
{
	if ( m_pos + sizeof(char) > m_size ) return false;
	value = m_buffer[m_pos];
	m_pos += sizeof(char);

	return true;
}

bool Stream::GetData( short &value )
{
	if ( m_pos + sizeof(short) > m_size ) return false;
//  	short value = Stream2I(&m_buffer[m_pos], sizeof(short));
	OutStream(value, &m_buffer[m_pos], sizeof(short), m_byteOrder);
	m_pos += sizeof(short);

	return true;
}

bool Stream::GetData( unsigned short &value )
{
	if ( m_pos + sizeof(short) > m_size ) return false;
//  	unsigned short value = Stream2I(&m_buffer[m_pos], sizeof(short));
	OutStream(value, &m_buffer[m_pos], sizeof(short), m_byteOrder);
	m_pos += sizeof(short);

	return true;
}

bool Stream::GetData( int &value )
{
	if ( m_pos + sizeof(int) > m_size ) return false;
// 	int value = Stream2I(&m_buffer[m_pos], sizeof(int));
	OutStream(value, &m_buffer[m_pos], sizeof(int), m_byteOrder);
	m_pos += sizeof(int);

	return true;
}

bool Stream::GetData( unsigned int &value )
{
	if ( m_pos + sizeof(int) > m_size ) return false;
// 	unsigned int value = Stream2I(&m_buffer[m_pos], sizeof(int));
	OutStream(value, &m_buffer[m_pos], sizeof(int), m_byteOrder);
	m_pos += sizeof(int);

	return true;
}

bool Stream::GetData( int64 &value )
{
	if ( m_pos + sizeof(int64) > m_size ) return false;
// 	int64 value = Stream2I(&m_buffer[m_pos], sizeof(int64));
	OutStream(value, &m_buffer[m_pos], sizeof(int64), m_byteOrder);
	m_pos += sizeof(int64);

	return true;
}

bool Stream::GetData( uint64 &value )
{
	if ( m_pos + sizeof(int64) > m_size ) return false;
// 	uint64 value = Stream2I(&m_buffer[m_pos], sizeof(int64));
	OutStream(value, &m_buffer[m_pos], sizeof(int64), m_byteOrder);
	m_pos += sizeof(int64);

	return true;
}

bool Stream::GetData(void* stream, int &size)
{
	if ( !GetData( size ) ) return false;
	if ( 0 >= size || m_pos + size > m_size ) 
	{
		m_pos -= sizeof(int);
		return false;
	}
	memcpy( stream, &m_buffer[m_pos], size );
	m_pos += size;
	
	return true;
}

bool Stream::GetData(std::string &str)
{
	int size = 0;
	str = "";
	void *stream = GetPointer(size);
	if ( NULL == stream ) return false;
	if ( 0 < size )
	{
		str = std::string((char*)stream, size - 1);
	}

	return true;
}

void* Stream::GetPointer( int &size )
{
	if ( !GetData( size ) ) return NULL;
	if ( 0 >= size || m_pos + size > m_size ) 
	{
		m_pos -= sizeof(int);
		return NULL;
	}
	void *stream = &m_buffer[m_pos];
	m_pos += size;
	
	return stream;
}

bool Stream::GetData(float &value)
{
	if ( m_pos + sizeof(float) > m_size ) return false;
	memcpy( &value, &m_buffer[m_pos], sizeof(float) );
	m_pos += sizeof(float);

	return true;
}

bool Stream::GetData(double &value)
{
	if ( m_pos + sizeof(double) > m_size ) return false;
	memcpy( &value, &m_buffer[m_pos], sizeof(double) );
	m_pos += sizeof(double);

	return true;
}

//将一个整数存储到字节流buf，按照byteOrder字节序(0网络序，1大端，2小端)
void Stream::I2Stream( unsigned char *buf, uint64 value, int size )
{
	if ( local == m_byteOrder ) 
	{
		if ( 1 == size ) buf[0] = value;
		else if ( 2 == size ) 
		{
			unsigned short val = value;
			memcpy(buf, &val, size);
		}
		else if ( 4 == size ) 
		{
			unsigned int val = value;
			memcpy(buf, &val, size);
		}
		else memcpy(buf, &value, size);
	}
	else if ( netOrder == m_byteOrder )
	{
		if ( size > 8 ) return;
		int high = value >> 32;
		int low = value;

		if ( 4 < size )
		{
			int netOrder = htonl(high);		
			memcpy(buf, &netOrder, sizeof(int));
			netOrder = htonl(low);
			memcpy(&buf[sizeof(int)], &netOrder, sizeof(int));
		}
		else 
		{
			int netOrder = htonl(low);
			netOrder = netOrder >> (4 - size) * 8;
			memcpy(buf, &netOrder, size);
		}
	}
	else if ( bigOrder == m_byteOrder )itomemBig(buf, value, size);
	else itomemSmall(buf, value, size);
}

//将buf字节流专程整数，按照byteOrder字节序(0网络序，1大端，2小端)
uint64 Stream::Stream2I( unsigned char *buf, int size )
{
	if ( local == m_byteOrder )
	{
		if ( 1 == size ) return buf[0];
		else if ( 2 == size ) 
		{
			unsigned short value;
			memcpy(&value, buf, size);
			return value;
		}
		else if ( 4 == size ) 
		{
			unsigned int value;
			memcpy(&value, buf, size);
			return value;
		}
		else 
		{
			uint64 value;
			memcpy(&value, buf, size);
			return value;
		}
	}
	else if ( netOrder == m_byteOrder )
	{
		if ( size > 8 ) return 0;

		uint64 value = 0;
		if ( 4 < size )
		{
			unsigned int high = ntohl(*((unsigned int*)buf) );
			unsigned int low = ntohl(*((unsigned int*)&buf[sizeof(int)]) );
			value = high;
			value = value << 32;
			value += low;
		}
		else 
		{
			char stream[4];
			int i = 0;
			for ( i = 0; i < 4 - size; i++ ) stream[i] = 0;
			int j = 0;
			for ( ; i < 4; i++ ) stream[i] = buf[j++];
			unsigned int low = ntohl(*((unsigned int*)stream) );
			value = low;
		}

		return value;
	}
	else if ( bigOrder == m_byteOrder ) return memtoiBig(buf, size);
	else return memtoiSmall(buf, size);
}

void Stream::StartAdd()
{
	m_building = true;
	m_size = sizeof(int);
}

void Stream::StartGet()
{
	m_building = false;
	m_pos = sizeof(int);

//	m_size = Stream2I(m_buffer, sizeof(int));
	OutStream(m_size, m_buffer, sizeof(int), m_byteOrder);
	if ( 0 > m_size || m_size > m_space )
	{
		m_size = -1;
	}
}

}
