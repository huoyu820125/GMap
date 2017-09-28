#include "../../../include/frame/netserver/HostData.h"
#include "../../../include/mdk/atom.h"
#include "../../../include/mdk/mapi.h"

namespace mdk
{

HostData::HostData()
{
	m_autoFree = false;
	m_pHost = &m_hostRef;//ָ�������
	m_refCount = 0;
}

HostData::~HostData()
{
	if ( m_autoFree )
	{
		m_autoFree = false;
		m_pHost = &m_hostRef;
		return;
	}
	NetHost empty;
	m_hostRef = empty;//�ͷ�host����
}

void HostData::Release()
{
	/*
		NetConnect::GetData()�����߼�����
		m_refCount > 0���ﲻ���ͷ��ڴ棬����GetData()������û��ϵ

		m_refCount = 0ʱ����GetData()������һ������SetData()�����������п�����SetData(NULL)Ҳ�п�����SetData(������)
		��ΪSetData(NULL)֮��GetData�Ͳ�����ͨ��NULL == m_pHostData���

		if oldCount = AtomDec(&m_refCount, 1); �����
			oldCount = 0����ȥִ���������ڴ��ͷ�
			m_refCount = -1;
			GetData()�е�if ( -1 == AtomAdd(&m_pHostData->m_refCount, 1) )��ִ�У��᷵���棬return NULL
			���᷵��Ұָ��
		else GetData()�е�if ( -1 == AtomAdd(&m_pHostData->m_refCount, 1) )�����
			m_refCount = 1;
			GetData()�᷵���ڴ��ַ
			Ȼ�������oldCount = AtomDec(&m_refCount, 1);ִ��
			m_refCount = 0;
			oldCount = 1 �����ͷ��ڴ棬ֱ��return��
			GetData()���صĵ�ַ����Ұָ�룬���������Ѿ���Hostû�й�����
	*/
	int oldCount = AtomDec(&m_refCount, 1);//����-1�����ؼ�֮ǰ��ֵ��
	if ( 0 > oldCount ) mdk_assert(false);//�ظ��ͷ�
	if ( 0 != oldCount ) return;
	/*
		��1Release()û��Lock,�᲻�����ͷ�֮ǰ���з���GetData()���ã�����Ұָ���أ�
		�����ܣ���ΪGetData()��ǰ����SetData()����SetData()�Ὣ���ü������ӣ�
		�������û�ִ��SetData(NULL)�ͷ�����֮ǰ�����ü����ǲ����ܹ�0�ġ�
		����������뵽����
		
		��Ȼ����������˵�����ⲿ�Ѿ�û��HostData���κ����ô��ڣ�����GetData()ֻ�᷵��NULL

		��2Release()û��Lock,�᲻�����ͷ�֮ǰ���з���SetData()���ã���Ұָ��󶨸�һ��NetHost�أ�
		�����ܣ���ΪҪ��HostData���ݸ�SetData������Ҫ����һ�����ã�Ҳ����˵���ü�������Ϊ0
		Ҳ�Ͳ����ܽ���������
	*/
	//oldCount��ʱӦ��=-1���������Ѿ����ͷ�
	//֮��Ӧ������Release()���ã��������release���þ����ظ��ͷţ�Ӧ�õ���mdk_assert����
	ReleaseInterface();
}

void HostData::ReleaseInterface()
{
	delete this;
}

NetHost HostData::GetHost()
{
	AutoLock lock(&m_lockHostRef);
	return *m_pHost;//Ҫôָ����Ч������Ҫôָ��m_hostRef������Ч��������Ч��m_pHost������NULL
}

void HostData::SetHost( NetHost *pHost )
{
	AutoLock lock(&m_lockHostRef);
	m_hostRef = *pHost;
}

}