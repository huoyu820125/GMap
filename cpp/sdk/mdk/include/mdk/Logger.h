// Logger.h: interface for the Logger class.
//
//////////////////////////////////////////////////////////////////////

#ifndef MDK_LOGGER_H
#define MDK_LOGGER_H

#include <stdio.h>
#include <string>
#include "Lock.h"

namespace mdk
{

class Logger  
{
public:
	Logger();
	Logger(const char *name);
	virtual ~Logger();

public:
	//������־������־�������浽"log/����/"��Ŀ¼��
	//ֻ������1��,�Ժ󶼽�ʧ��
	//������캯���Ѿ���������־��,��˷�������������
	bool SetLogName( const char *name );
	void SetPrintLog( bool bPrint );//д��־ʱ���Ƿ�ͬʱ��ӡ������̨
	bool Info( const char *findKey, const char *format, ... );//�����Ϣ�������ʽΪ ʱ��+�ո�+(findKey)+�ո�+(Tid:�߳�ID)+�ո�+����������Ϣ
	bool StreamInfo( const char *findKey, unsigned char *stream, int nLen, const char *format, ...  );//�������������������Ϣ, stream���ݽ�����format��Ϣ֮���������stream:Ϊ���
	void SetMaxLogSize( int maxLogSize );//���õ�����־�ļ����ߴ磬��λM
	void SetMaxExistDay( int maxExistDay );//������־�ļ��������������������Ϊ30����ౣ�����30�����־
	//ɾ��nDay��ǰ����־
	void DelLog( int nDay );
private:
	bool OpenRunLog();
	bool CreateLogDir();//����ʼ��,ֻ�ܵ���1��
	bool CreateFreeDir(const char* dir);//�������ɣ����Ȩ�ޣ�Ŀ¼
	void RenameMaxLog();//����������־
	void FindDelLog(char * lpPath, int maxExistDay);

private:
	bool m_isInit;//�Ƿ��ʼ��
	bool m_bPrint;//�����־ʱ,�Ƿ�ͬʱ��ӡ������̨
	Mutex m_writeMutex;
	bool m_bRLogOpened;//������־�ļ��Ƿ��
	//������־��ǰ����
	short m_nRunLogCurYear;
	unsigned char m_nRunLogCurMonth;
	unsigned char m_nRunLogCurDay;
	FILE *m_fpRunLog;//��־�ļ�ָ��
	std::string m_name;//��־��
	std::string m_runLogDir;//��־Ŀ¼
	int m_maxLogSize;//��־�ļ����ߴ�
	int m_maxExistDay;
	int m_index;//��־���
	char *m_exeDir;
};

}//namespace mdk

#endif // MDK_LOGGER_H
