#ifndef MDK_NETHOST_H
#define MDK_NETHOST_H

#include "../../../include/mdk/FixLengthInt.h"
#include <string>

namespace mdk
{
class HostData;
class NetConnect;
class Socket;
/**
	����������
	��ʾһ������������������˽�й��캯����ֻ�������洴�����û�ʹ��

	��NetHost�����ƹ淶
	������һ����������࣬��������ָ�룬����ͨ�����Ʋ�����NetHost������һ��NetConnectָ�롣
	���Բ�Ҫ���Ƶ�ַ�����ã���Ϊ�������ü�������ı䣬��ɿ��ܷ���һ���Ѿ����ͷŵ�NetConnectָ��
	��������
	NetHost *pHost = &host;//���Ƶ�ַ�����ü�������ı�
	vector<NetHost*> hostList;
	hostList.push_back(&host);//���Ƶ�ַ�����ü�������ı�
	��ȷ������
	NetHost safeHost = host;//���ƶ������ü���������1����safeHost����֮ǰ��NetConnectָ��ָ����ڴ���Բ��ᱻ�ͷ�
	vector<NetHost> hostList;
	hostList.push_back(host);//���ƶ������ü���������1���ڶ����hostList��ɾ��֮ǰ��NetConnectָ��ָ����ڴ���Բ��ᱻ�ͷ�
*/
class NetHost
{
	friend class NetConnect;
public:
	NetHost();
	virtual ~NetHost();
	NetHost(const NetHost& obj);	
	NetHost& operator=(const NetHost& obj);
	/*
		����Ψһ��ʶ
		��V1.79��ǰ��ʵ�ʾ�������������ӵ�int�����������ֱ��ʹ��socket���api��������socket��io��close��
		��Ϊcloseʱ���ײ���Ҫ�������������ֱ��ʹ��socketclose()����ײ����û����ִ��������,������Ӳ�����
		io�����ײ��Ѿ�ʹ��io�������ֱ��ʹ��api io������io��������һ���ײ�io���������������ݴ���

		��V1.79��ʼ�����һ��������int64ֵ��int����޹�
			id��Χ??�ף�����??������֮��id��תһȦ��������Ҫʱ���Լ��ʮ��
			���Ե�ĳ�����ӳ�����ʮ�����ߵļ�������£��ſ��ܵ���2������ID��ͬ
	*/	
	int64 ID();

	/*
		���ÿͻ�����
		������߳�ͬʱSetData()�������̰߳�ȫ�����δ��֪
		�������κη�ʽ�����������̰߳�ȫ��

		autoFree = true ����ģʽ
			HostData�����������ɿ�ܹ����û�����Ҫ��ʽ����HostData.Release()��Ҳ����Ҫ����SetData(NULL)
			ʹ��˵����
				1.���ܽ�HostDataָ�븳�������̷߳���
				2.����copy���ƣ�HostDataֻ�����������ƶ���������֮��ʹ��,�����ǲ���ȫ��
				3.����SetData()������������HostData���ͷŹ����Ѿ��ƽ�������ˣ���Ҫ��ִ��HostData���ͷŲ���
				4.һ������SetData()������������֮����κ�SetData()����������������SetData(NULL)��
					��Ϊ��ģʽ�£����е�copy���ǲ��������ü����ģ�һ�����������
					�û��޷�֪��ʲôʱ��HostData���ǿ��԰�ȫɾ����

			ע�⣺
			��ܻ���NetConnect���ͷ�ʱ�����û��ͷ�����

			��ʹ������2˵��
			����ģʽ�£�����copy���ƣ�HostDataֻ�����������ƶ���������֮��ʹ��,�����ǲ���ȫ��
			����HostData *pData = host.GetData();
			pData�Ǵ�host��������еõ��ģ���ôpDataֻ����host���ͷ�ǰʹ�ã�
			�����뿪host����������NetConnect���ü����п���Ϊ0���ײ��п��ܻ��ͷ�NetConnect��
			����pDataָ��Ұָ��
			�������´��뽫���ܷ���Ұָ��
			OnMessage( NetHost &host )
			{
				HostData *pData;
				if (...)//����Ϣת��������host
				{
					NetHost otherHost = hostmap.find( ����hostid );//��map���ҵ�����host
					//Σ�ղ�������HostData���Ƹ�һ������������ָ�룬pData��������>otherHost��������
					pData = otherHost.GetData();
					ʹ��pData
					�뿪otherHost������
				}
				pData���ܱ��Ұָ��
				��ΪotherHost�Ѿ���ʧ��otherHost��Ӧ�������п��ܱ�����һ��OnClose()�߳����ͷţ�
				����pDataһ���ͷ�
			}

		autoFree = false ����ģʽ
			HostData�������������û����й�����ܲ��ܣ���Ҫ�û���ʽ����HostData.Release()
			ʹ��˵����
				���������ͬһ��host���SetData������ͬ���ݣ������δ��֪
				1.ÿ��GetData()֮�������˼ǵ�Release()
				2.Ϊÿ����Ҫ����HostData���̣߳�����1��GetData()�����ר����HostDataָ�룬
					�����̷߳������ʱ������Release()�ͷŷ��ʣ��������ʵ��߳̽��������ͷ�����
				3.���Ӳ����˺󣬵���SetData(NULL)��������������Ĺ������������޷��ͷ�������Ӧ������

			ע�⣺
				GetData()��������1�����ã�����Ҫ�����ͷţ���ҪGetData()���ô���+1��release()
				���磺
					HostData *pData = new HostData();
					host.SetData(pData);
					����һ���ط�
					HostData *pData = host.GetData();
					ʵ���Ͼʹ�����2������HostData�ĵط�
					GetData()1����new��������1����������ҪRelease()2��
				-------------------------------------------------------------------------------------------
				����HostData�ڲ�������NetHost��һ�ݸ���
				��ʹ��SetData(NULL)���������NetHost�Ĺ���,��HostData�������ͷ�֮ǰ��
				NetConnect��Զ���ᱻ����ͷ�.
				����������Ч�Ժ���ҪSetData(NULL)��HostData.Release()�����ͷ�
				SetData(NULL)�ᵼ��NetConnect������ͷ�
				HostData.Release()ֻ�������һ��HostData.Release()��ʱ���Ż��ͷ�NetConnect

				����GetData()��֤��HostData�ķ��ʰ�ȫ������
				�û��������κεط�ʹ��HostDataָ�룬ֻҪ���ָ���ǳ���GetData()
	*/
	void SetData( HostData *pData, bool autoFree = true );
	//ȡ�ÿͻ����ݣ����������򣬲ο�SetData()��2��ģʽ˵��
	HostData* GetData();
	/*
		�ӽ��ջ����ж�����
			������
				pMsg		��Ϣ������յ�����
				uLength		��Ҫ���յĳ���
				bClearCache	�Ƿ񽫽��յ������ݴӻ����������trueɾ����false����
				���籨�ĸ�ʽ��2byte���ݳ���+��������
				OnMsg()�ڽ����߼�����
					1.Recv(msg, 2, true);
					2.����msg�õ����ݳ��ȣ�����Ϊ256
					3.Recv(msg, 256, true)
					��
					���3.���ﷵ��false����ʾʵ�ʵ�������<256��������ȡ
					
					��ʱ���û���2��ѡ��
					ѡ��1��
					ѭ��ִ��Recv ֱ���ɹ��������sleep��CPUֱ��100%�����sleep����ӦЧ�ʽ���
					ѡ��2��
					��256����������ֱ��return�˳�OnMsg���´�OnMsg����ʱ���ٳ���Recv
					�ŵ㣬û��sleep,����CPU
					ȱ�㣺�û�����������֯���û���ҪΪ��������ά��һ��int����������ճ��ȣ�
					Ҳ�����û���Ҫ�Լ�ά��һ���б������ӶϿ�ʱ��Ҫ���б�ɾ������������
					
					����false��bClearCache��������������
					1.Recv(msg, 2, false);
					2.����msg�õ����ݳ��ȣ�����Ϊ256
					3.Recv(msg, 256+2, true)//�������ĳ�����256+2
						���Recv�ɹ���ֱ�Ӵ���
						���Recvʧ�ܣ���ʾ�������ݲ�������Ϊ1.���ﴫ����false��
						���ĳ�����Ϣ����ӽ��ջ�����ɾ�������ԣ��û�����ֱ��return�˳�OnMsg,
						�´�OnMsg����ʱ�������Դ������϶����������ݳ�����Ϣ
			
			����ֵ��
			���ݲ�����ֱ�ӷ���false
			��������ģʽ�������Ѿ����û��������Ϣ�ȴ�����Ϣ����ʱ����OnMsg������
	*/
	bool Recv(unsigned char* pMsg, unsigned int uLength, bool bClearCache = true );
	/**
		��������
		����ֵ��
			��������Чʱ������false
	*/
	bool Send(const unsigned char* pMsg, unsigned int uLength);
	void Close();//�ر�����
	bool IsServer();//������һ������
	void InGroup( int groupID );//����ĳ���飬ͬһ�������ɶ�ε��ø÷���������������
	void OutGroup( int groupID );//��ĳ����ɾ��
	/*
		������ַ
		�������NetServerϣ���õ��Է���ַ��Ӧ�õ��ñ�������������GetServerAddress
		��ΪNetHost��ʾ�ľ��ǶԷ�����������NetHost��������ַ���ǶԷ���ַ
		if ������ʹ��127.0.0.1��������ȡ����ַ127.0.0.1
		if ������ʹ��������ַ��������ȡ��������ַ
		if ������ʹ��������ַ��������ȡ��������ַ
	 */
	void GetAddress( std::string &ip, int &port );//������ַ
	/*
		��������ַ
		�����ϣ��֪���Է��������ӵ��Լ����ĸ��˿ڣ�Ӧ�õ��ñ�������������GetAddress]
		��ΪGetAddress��ʾ���ǶԷ�
		if ������ʹ��127.0.0.1��������ȡ����ַ127.0.0.1
		if ������ʹ��������ַ��������ȡ��������ַ
		if ������ʹ��������ַ��������ȡ��������ַ
	 */
	void GetServerAddress( std::string &ip, int &port );
	/*
		ȡ������Ϣ
		���ǵ���NetServer::Connect()ʱ����ĵ�3������
	*/
	void* GetSvrInfo();
private:
	NetConnect* m_pConnect;//���Ӷ���ָ��,����NetConnect��ҵ���ӿڣ�����NetConnect��ͨ�Ų�ӿ�
	
};

}  // namespace mdk
#endif//MDK_NETHOST_H
