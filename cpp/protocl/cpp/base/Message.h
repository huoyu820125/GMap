#pragma once
#include "Stream.h"

namespace net
{
/*
	ͨ��Э��ӿڻ���
	ʹ��˵��
		�������ڹ����е���BindSetting��buffer��������󳤶ȡ�ģ��ID���ð���
		������������������public����

		���ձ���
			Message������	msg;
			recv( msg, msg.HeaderSize() );//���ձ���ͷ�����浽msg��
			if ( 0 > msg.Size() ) �Ѿ����Է��ر����ܳ��ȣ���������ͷ,< 0��ʾ����ͷ�Ƿ�
			{
				������
				return;
			}
			unsigned char *buffer = msg;//ȡ�������ַ
			recv( &buffer[msg.HeaderSize()], msg.Size() - msg.HeaderSize() );//���ձ������ݣ����浽msg�У��ӱ���ͷ֮��
			if ( ���msg.Id() )
			{
					msg.��Ӧҵ��Ľ������(...);//������ʵ��
					��ҵ��
			}

		����������
			Message������	msg;
			msg.��Ӧҵ����������(...);//������ʵ��
			send( msg, msg.Size() );//���ͱ���
*/
class Message : public Stream
{
public:
	Message(void);
	virtual ~Message(void);

public:
	virtual int HeaderSize();//����ͷ����
	bool ReadHeader();//��ȡ����ͷ
	//����ʱ����ֱ�Ӵ��ݸ�send�ӿڣ�����ʱ����ֱ�ӽ����ݸ�����msg��������copy
	operator unsigned char*();
	operator char*();
	operator void*();
	virtual unsigned char MoudleId();//ģ��ţ�����ͬʱ���ض��sdkʱЭ����ظ�
	virtual short Id();//����Э���
	virtual bool IsResult();//�ǻ�Ӧ����

	virtual bool Parse(){ return false; };//��������
	virtual bool Build( bool isResult = false ){ return false; };//���챨�ģ�isResult=true�����Ӧ����
protected:
	//��Э������:buffer����󳤶ȡ�ģ��Id��������sdkЭ����ظ�
	void BindSetting( unsigned char *buffer, int maxSize, char byteOrder, unsigned char moudleId );//������������
	virtual void SetId( short id, bool isResult = false );//����id

private:
	unsigned char	m_moudleId;	//ģ��Id
	short			m_id;		//Э���
	char			m_isResult;	//�ǻ�Ӧ����
};

}
