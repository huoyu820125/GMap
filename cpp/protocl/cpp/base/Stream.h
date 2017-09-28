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
	//�ֽ���
	enum ByteOrder
	{
		netOrder = 0,
		bigOrder = 1,
		smallOrder = 2,
		local = 3,
	};

/**
* ����
*/
class Stream
{
public:
	Stream();
	virtual ~Stream();

public:
	void Bind(unsigned char *buffer, unsigned int space);//��һ�����������ڴ�֮����ת��
	void SetByteOrder( ByteOrder order );//�����ֽ���
	operator unsigned char*();
	operator char*();
	operator void*();
	int Size();//ʵ�ʳ��ȣ��Ƿ�ֵ��С��0
//////////////////////////////////////////////////////////////////////////
//��Ӳ���,�ɹ�����true��ʧ�ܷ���false
	/**
	 * ����1����������Ϣ��,char��unsigned char��int�ȸ������͵�����
	 * �ɹ�����true��ʧ�ܷ���false
	 * ��ֱ�Ӵ��ݳ��������Զ�ƥ�䵽4~nbyte����С������
	 * ����AddData( 1 )����Ϊ�ǲ���һ��int��ռ4byte�����ϣ��ֻռ1byte����ʹ��AddData((char)1)
	 * ����AddData( 0xffffffff )����Ϊ�ǲ���һ��unsigned int��ռ4byte
	 * ����AddData( 0xffffffff ff)����Ϊ�ǲ���һ��int64��ռ8byte
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
//ȡ�ò������ɹ�����true��ʧ�ܷ���false
	/**
	* ������ȡ��һ������char��unsigned char��int�ȸ������͵�����
	* �ɹ�����true��ʧ�ܷ���false
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
	 * ������ȡ�ñ䳤����
	 * 
	 * 
	 * stream�������Ļ���
	 * size���뻺������������ʵ�ʶ�����С
	 * 
	 */
	bool GetData(void* stream, int &size);
	bool GetData(std::string &str);
	/**
	 * Ϊ����ȥCopy��ֱ�Ӵ�����ȡ�ñ䳤�����ָ���ַ
	 * �ɹ����ض���ָ�룬ʧ�ܷ���NULL
	 * uSize���ص�ַ��С
	*/
	void* GetPointer( int &size );
	bool GetData(float &value);
	bool GetData(double &value);

protected:
	//�����ʹ�������size��ʾ���ͳ���char short int int64
	void I2Stream( unsigned char *buf, uint64 value, int size );
	//������ȡ�����Ͳ�����size��ʾ���ͳ���char short int int64
	uint64 Stream2I( unsigned char *buf, int size );

	void StartAdd();
	void StartGet();

protected:
	unsigned char*	m_buffer;		//�󶨵Ļ�����
	unsigned int	m_space;		//�����С
	int				m_size;			//��ʵ�ʳ���
	int				m_pos;			//�α�λ��
	ByteOrder		m_byteOrder;	//����ʹ�õ��ֽ�˳
	bool			m_building;		//���Ĺ�����
};

}
