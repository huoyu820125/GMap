// MD5Helper.h: interface for the MD5Helper class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MD5_HELPER_H
#define MD5_HELPER_H

#include <string>

typedef unsigned char *MD5_POINTER;

//@MD5-hash context.
typedef struct MD5_CONTEXT 
{
	/** state (ABCD) */
	unsigned long int state[4];   	      

	/** number of bits, modulo 2^64 (lsb first) */
	unsigned long int count[2];

	/** input buffer */
	unsigned char buffer[64];
}MD5_CONTEXT;

class MD5Helper  
{
public:
	MD5Helper();
	virtual ~MD5Helper();
	std::string HashString( const void* input, int size );//ȡ���ַ�����ʽ��hashֵ
	unsigned int HashValue( const void* input, int size );//ȡ����ֵ��ʽ��hashֵ

protected:
	void Init();
	void Update( unsigned char *input, unsigned int inputLen );
	void Final( unsigned char digest[16] );
	std::string HashToString( unsigned char *data );
	unsigned int DJBHash( const unsigned char *buf, int len );

	void Encode( unsigned char *output, unsigned long int *input, unsigned int len );
	void Memset( MD5_POINTER output,int value,unsigned int len );
	void Memcpy( MD5_POINTER output, MD5_POINTER input, unsigned int len );
	void Transform( unsigned long int state[4], unsigned char block[64] );
	void Decode( unsigned long int *output, unsigned char *input, unsigned int len );

	/**
	 * MD5 context
	 */
	MD5_CONTEXT ctx;
};

#endif // MD5_HELPER_H
