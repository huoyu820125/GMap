#pragma once
#include "Stream.h"

namespace net
{
/*
	通信协议接口基类
	使用说明
		派生类在构造中调用BindSetting将buffer、报文最大长度、模块ID设置绑定了
		或者派生类重载所有public方法

		接收报文
			Message派生类	msg;
			recv( msg, msg.HeaderSize() );//接收报文头，保存到msg中
			if ( 0 > msg.Size() ) 已经可以返回报文总长度，包含报文头,< 0表示报文头非法
			{
				错误处理
				return;
			}
			unsigned char *buffer = msg;//取出缓冲地址
			recv( &buffer[msg.HeaderSize()], msg.Size() - msg.HeaderSize() );//接收报文数据，保存到msg中，接报文头之后
			if ( 检查msg.Id() )
			{
					msg.对应业务的解包方法(...);//派生类实现
					做业务
			}

		发送请求报文
			Message派生类	msg;
			msg.对应业务的组包方法(...);//派生类实现
			send( msg, msg.Size() );//发送报文
*/
class Message : public Stream
{
public:
	Message(void);
	virtual ~Message(void);

public:
	virtual int HeaderSize();//报文头长度
	bool ReadHeader();//读取报文头
	//发送时用于直接传递给send接口，接收时用于直接将数据复制入msg，避免多次copy
	operator unsigned char*();
	operator char*();
	operator void*();
	virtual unsigned char MoudleId();//模块号，避免同时加载多个sdk时协议号重复
	virtual short Id();//返回协议号
	virtual bool IsResult();//是回应报文

	virtual bool Parse(){ return false; };//解析报文
	virtual bool Build( bool isResult = false ){ return false; };//构造报文，isResult=true构造回应报文
protected:
	//绑定协议设置:buffer、最大长度、模块Id，避免多个sdk协议号重复
	void BindSetting( unsigned char *buffer, int maxSize, char byteOrder, unsigned char moudleId );//绑定派生类设置
	virtual void SetId( short id, bool isResult = false );//设置id

private:
	unsigned char	m_moudleId;	//模块Id
	short			m_id;		//协议号
	char			m_isResult;	//是回应报文
};

}
