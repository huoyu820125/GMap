#ifndef SERVER_EX_H
#define SERVER_EX_H

#include "mdk/include/frame/netserver/NetServer.h"
#include "mdk/include/mdk/Signal.h"
#include "mdk/include/mdk/Lock.h"
#include "mdk/include/mdk/Executor.h"
#include "mdk/include/mdk/ConfigFile.h"
#include "Console.h"

#include <map>
#include <vector>
#include <string>

namespace mdk
{

class ServerEx : public NetServer
{
public:
	ServerEx(void);
	virtual ~ServerEx(void);

	//////////////////////////////////////////////////////////////////////////
	//ҵ���̶߳�ռ����
public:
	//���ù����߳�����
	void SetWorkThreadCount(int nCount);
	/*
		���ù����̷߳��ʶ�����ÿ�������̶߳�ռһ������
		objListԪ�����������빤���߳�������ͬ��ȷ��ÿ�������߳�1�����󲻶಻��
		������SetWorkThreadCount��Ĭ����16�������߳�
		���������ã������ٵ���SetWorkThreadCount,�������̶��Ա���

		objList����Ķ�����뱣֤��~ServerEx()ǰ�����ᱻ�ͷ�
		�����new�����ģ���Ҫ����freeFun��pObj����������~ServerEx����ͷŶ���
		���磺
		class AB
		{
			public:
			AB(){}
			virtual ~AB(){}
			void* RemoteCall freeData(void *p)
			{
				A *pA = (A*)p;
				delete pA;

				return NULL;
			}
		};

		AB *p = new AB();
		SetThreadsObjects(a, mdk::Executor::Bind(&AB::freeData), p);
	*/
	void SetThreadsObjects( const std::vector<void*> &objList, 
							MethodPointer freeFun = NULL, void *pObj = NULL );

	//���ص��ù����̶߳�ռ�Ķ���
	void* SafeObject();

protected:
	//Ϊһ���̷߳��䰲ȫ����
	void AllocSafeObject();

private:
	void* RemoteCall OnWorkStart(void*);

private:
	int m_workThreadCount;//ҵ���߳�����
	std::map<uint64, void*> m_TObjectMap;//�����߳�ID������ռ����ӳ��
	std::vector<void*> m_objects;//����������̵߳Ķ���
	std::vector<void*>::iterator m_nextObject;//��һ��δ�������
	Mutex	m_startLock;//OnWorkStart��
	Signal	m_sigFinished;//ҵ���̳߳��������
	bool	m_threadObjSeted;//�̶߳���������
	//�̶߳����ͷŷ���
	MethodPointer m_freeFun;
	void *m_pFreeFunObj;

	//////////////////////////////////////////////////////////////////////////
	//�ֲ�ʽ����Ψһid
protected:
	/*
		ʹ��˵��
		����OnlyId.cfg���ý��ΨһId
		��������ǰ����1��InitId(�����ļ���)
		�Ժ�ÿ�ε���CreateId()�Ϳ��Բ�����Ⱥ��ȫ��Ψһid
		����SaveId()�������´ο���idд�����
	*/
	//��ʼ��id
	void InitId(const std::string &fileName);
	//����ȫ��Ψһid,������SaveId()��ʱд����
	bool CreateId( uint64 &id );
	//д����
	//now=true����д
	//now=false ÿʹ��10���id,���ϴδ���id���뱾�δ�������5��,��д1�δ���
	void SaveId( bool now = true );

private:
	uint16 m_nodeId;
	uint64 m_searialNo;
	ConfigFile *m_pCfg;

	//////////////////////////////////////////////////////////////////////////
	//����������֧��
public:
	void SetPrompt( char *prompt );//������ʾ�����������κ�ʱ����ã�Ϊ���޸���ʾ��
	void StartCommand();//��ʼ��������������
	void StopCommand();//ֹͣ������

protected:
	char* RemoteCall CommandAcceptor(std::vector<std::string> *cmd);
	virtual char* OnCommand(const std::vector<std::string> &cmd);

private:
	Console	m_console;
};

}

#endif	//SERVER_EX_H
