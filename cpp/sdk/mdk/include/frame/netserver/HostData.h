#ifndef HOST_DATA_H
#define HOST_DATA_H
#include "../../../include/mdk/Lock.h"
#include "../../../include/frame/netserver/NetHost.h"
#include "../../../include/mdk/Lock.h"

namespace mdk
{

class NetHost;
/*
	��������
	���NetHost�����ݻ���
	��ʾ������ص�ҵ�����ݣ����û����������ж������ҵ�����ݡ�
	ҵ�����ݽ��鶨���public��������ѭoop�������Ա����ͨ���������ʵĹ���
	��Ϊ����Ŀ�ľ������������NetHost�ϵ����ݽṹ�����ݽṹ����ֱ�ӷ������ݵģ�
	����Ӧ�ðѸ����͵������ݽṹ������
*/
class HostData
{
friend class NetConnect;
public:
	HostData();
	virtual ~HostData();
	/*
		��ȫ���ͷŶ���
		�������ü���
		Ȼ�����ReleaseInterface�����ͷ�
	*/
	void Release();
	/*
		����NetHost���󣬶�������ָ�룬
		����NetHost��copy���ƹ���
	*/
	NetHost GetHost();

protected:
	/*
		����HostData����Ĵ�����ʽ��������new��malloc()���ڴ�ء�����أ����շ�ʽ������ͬ
		������Ҫ�û�ʵ���ͷŷ��������߿������ͷ�HostData����
		
		�����HostData�����Ǵ�new����������Բ�����дrelease()��Ĭ��ʹ��delete��ʽ���ն���HostData
	*/
	virtual void ReleaseInterface();
	void SetHost(NetHost *pHost);//�̰߳�ȫ���޸Ĺ�����host

private:
	bool m_autoFree;//true����ģʽ���ÿ���Զ��ͷ�HostData����false����ģʽ���û�������������
	NetHost m_hostRef;//host��һ�ݸ��ƣ��ڴ���ģʽʱ��ȷ��host����Ч��
	mdk::Mutex m_lockHostRef;//hostbian�仯��
	NetHost *m_pHost;
	/*
		���ü���
		���������������ʱ���˳�Ա������
		���û��Լ����������������ʱ������ȷ��release����ȷ��ʱ���ͷŶ���
	*/
	int m_refCount;
};

}
#endif //HOST_DATA_H