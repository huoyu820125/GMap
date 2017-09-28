// ConfigFile.h: interface for the ConfigFile class.
//
//////////////////////////////////////////////////////////////////////
/*
	��ʶ����ļ���ʽ��
	ע������#����//��ͷ
	������section�λ���
	��ʽΪ���۶������������ݣ�/������
	�����ڿ��Դ���������ո��Ʊ���������˵Ŀհ����Ʊ���ᱻ����
	���磺[ ser config ]����������"ser config"

	��������Ϊһ��һ��
	���ø�ʽΪkey=value
	���Դ���������ո��Ʊ��
	key��value�ַ��������а����Ŀո����Ʊ���������������˵Ŀո����Ʊ���ᱻ����
	����:"  ip list = \t 192.168.0.1 \t 192.168.0.2 \t "
	�������������
		key ="ip list"
		value="192.168.0.1 \t 192.168.0.2"

	ʹ�÷���
	�����ļ�����
	[ser config]
	ip = 192.168.0.1
	port = 8888
	[/ser config]

	ConfigFile cfg( "./test.cfg" );
	string ip = cfg["ser config"]["ip"];
	cfg["ser config"]["ip"] = "127.0.0.1";
	cfg["ser config"]["port"] = 8080;//����ֱ��ʹ��char*��string��int��float�Ƚ��и�ֵ
	cfg.save();
*/
#ifndef MDK_CONFIGFILE_H
#define MDK_CONFIGFILE_H

#include "FixLengthInt.h"

#include <stdio.h>
#include <string>
#include <map>

namespace mdk
{

class CFGItem
{
	friend class ConfigFile;
	friend class CFGSection;
public:
	bool IsNull();
	operator std::string();
	operator char();
	operator unsigned char();
	operator short();
	operator unsigned short();
	operator int();
	operator unsigned int();
	operator int64();
	operator uint64();
	operator float();
	operator double();
	CFGItem& operator = ( double value );
	CFGItem& operator = ( int value );
	CFGItem& operator = ( std::string value );
	//����ע�ͣ�����windows linux��һ��ʹ��\n��Ϊ���з�
	void SetDescription( const char *description );
	virtual ~CFGItem();
private:
	CFGItem();
	std::string m_value;
	std::string m_description;
	int m_index;
	bool m_valid;
};
typedef std::map<std::string,CFGItem> ConfigMap;

class CFGSection
{
	friend class ConfigFile;
public:
	CFGSection(const char *name, int index);
	~CFGSection();
public:
	//����д����ֵ
	CFGItem& operator []( std::string key );
	//����ע�ͣ�����windows linux��һ��ʹ��\n��Ϊ���з�
	void SetDescription( const char *description );
	bool Save(FILE *pFile);

private:
	std::string m_name;
	ConfigMap m_content;
	std::string m_description;
	int m_index;

};

typedef std::map<std::string,CFGSection> CFGSectionMap;
class ConfigFile  
{
public:
	ConfigFile();
	ConfigFile( const char *strName );
	virtual ~ConfigFile();
	
public:
	//����д����ֵ
	CFGSection& operator []( std::string name );
	//��ȡ����,ֻ�ܳɹ���ȡ1��,һ����ȡ�ɹ�,�Ͳ����ٶ�ȡ,�ٵ��ö�������false
	bool ReadConfig( const char *fileName );
	bool Save();
private:
	//���ļ�
	bool Open( bool bRead );
	//�ر��ļ�
	void Close();
	bool ReadFile();

private:
	std::string m_strName;
	FILE *m_pFile;
	CFGSectionMap m_sections;
};

}//namespace mdk

#endif // MDK_CONFIGFILE_H
