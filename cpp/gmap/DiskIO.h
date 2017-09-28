#ifndef DISKIO_H
#define DISKIO_H

#include "mdk_ex/File.h"

#include <vector>
#include <map>
#include "common/struct/Point.h"
#include "common/struct/Line.h"


//����io
//���ݳ־û��Ͷ�ȡ
/*
���ж�����ߵĲ�������ʹ��id��Ϊ����
��֧����������
����ϲ���Ҫ���������ֻ�����ԣ�û��id����������Ҫ�����̻�ȡ��
����ȷ��id��Χ��ͨ��id�������õ��ڴ棬Ȼ�������ڴ���ƥ���������
�����ڴ�������÷�Χ

�ļ��洢�ṹ
�洢�ļ�����
���������ļ����洢����id��Ӧ���������������ļ�����ʼλ��
���������ļ����洢��������
�������ļ����洢��id��Ӧ�������������ļ�����ʼλ��
�������ļ����洢������
�����ļ�����point/line+���(1~512)+.idx
�����ļ�����point/line+���(1~512)+.db

�����ļ���ʽ������or��id(8byte) + ������db�ļ��еĿ�ʼλ��(8byte) + ���ݳ���(4byte)
�����ļ���ʽ������״̬(1byte)0������1ɾ�� + ����(nbyte)

1000�򶥵㣬ÿ������2k���ֳ�512�������ļ��洢��ÿ���ļ���С38M
2000�����ߣ�ÿ����2k���ֳ�512�������ļ��洢��ÿ���ļ���С76M
*/
class GridStore;
class DiskIO
{
public:
	DiskIO();
	virtual ~DiskIO();
	void Init(GridStore *pStore);
	std::string DataDir();
	const char* CheckDataDir();//�������Ŀ¼
	//�������ش�����
	enum Error
	{
		noDataDir = 1,//û������Ŀ¼
		noPermissions = 2,//���ļ����ļ���Ȩ��
		noFile = 3,//�ļ�������
		errorParam = 4,//��������
		noDiskSpace = 5,//���̲���
		dataDamage = 6,//������
		noData = 7,//û������
	};
	const char* LoadPoints(std::map<bigint, Grid::Point*> &points);//��ȡ���ж���
	const char* LoadLines(std::map<bigint, Grid::Line*> &points);//��ȡ���б�

public:
	Grid::Result::Result SavePoint(Grid::Point &point);//���涥��
	Grid::Result::Result DelPoint(bigint pointId, bigint dataPos);//ɾ������
	Grid::Result::Result SaveLine(Grid::Line &line);//�����
	Grid::Result::Result DelLine(bigint lineId, bigint dataPos);//ɾ����
	//////////////////////////////////////////////////////////////////////////
	//��������
	//����һ������
	Grid::Result::Result CreatePoints(std::vector<Grid::Point*> &points);

protected:
	//����״̬
	enum DataState
	{
		normal = 0,//����
		del = 1,//ɾ��
	};

	class AutoCloseFile
	{
	public:
		AutoCloseFile( mdk::File *pFile );
		~AutoCloseFile();

		void Detach();//�����ļ����ļ��������Զ��ر�

	private:
		mdk::File *m_pFile;
	};

	const char* ReadPointFile(mdk::File &idxFile, std::map<bigint, Grid::Point*> &points);//��ȡ�ļ������ж���
	const char* ReadLineFile(mdk::File &idxFile, std::map<bigint, Grid::Line*> &points);//��ȡ�ļ������б�
	Grid::Result::Result ReadPoint(const bigint &dataPos, Grid::Point &point);
	Grid::Result::Result ReadLine(const bigint &dataPos, Grid::Line &line);
	//д�����������isUpdate = false������λ�ñ����idxPos
	Grid::Result::Result SaveIndex(mdk::File &idxFile, bool isUpdated, bigint &idxPos, bigint id, const bigint &dataPos, int size);
	/*
		��ʼ��������
		isUpdated = false��ʾ�����ǵ�һ�δ����������ʾ�޸���������,�����²���

		��������
			ֱ�ӽ��ļ��α��ƶ����ļ�ĩβ��׼��д�룬������ʼλ�ñ��浽dataPos
		�޸�����
			������ݳ����Ƿ񳬹������ݳ���
			����������򽫾���������Ϊɾ���������ļ��α��ƶ����ļ�ĩβ,��dataMoved����Ϊtrue��׼��д������
			�����ļ��α��ƶ������ݿ�ʼλ�ã�׼��д��
	*/
	Grid::Result::Result SaveDataStart(mdk::File &dbFile, bool isUpdated, bool &dataMoved, bigint &dataPos, int dataSize);
	//��ʼ��ȡ���ݣ��õ����ݳ���
	Grid::Result::Result ReadDataStart(mdk::File &dbFile, const bigint &dataPos, int &size);

	//�������ݵ��ļ�,�����д��ĩβ��������λ�ñ��浽dataPos
	Grid::Result::Result SaveData(mdk::File &dbFile, bigint &dataPos, char *data, int size, int oldSize, bool isCreate);

	//////////////////////////////////////////////////////////////////////////
	//����&�ֽ���֮��ת������
	/*
		ֻ����ȷ�����ݲ�ͣ�����ڴ���
		���Խ���־ò��������ݣ�һ���ǽ���ҵ����ģ���������->�ֽ�����ת����������ҵ���飬ֻ�������Խ�磨���ݳ���
		�ļ��ж��������ݣ����뾭��ҵ���飬�ſ��Խ����ڴ棬�����ֽ���->���ݵķ�����������Ҫ�������Խ�磬��Ҫ��ҵ����
	*/
	//�����б�ѹ���ֽ���
	bool Fields2Stream(char *stream, mdk::Map &fields, int &size, unsigned int space);
	//���ֽ�����ȡ�����б�
	bool Stream2Fields(mdk::Map &fields, int &size, char *stream, unsigned int space);
	//��������ѹ���ֽ���
	bool Point2Stream(char *stream, Grid::Point &point, char state, int &size, unsigned int space);
	//���ֽ�����ȡ��������
	bool Stream2Point(Grid::Point &point, char &state, int &size, char *stream, unsigned int space);
	//������ѹ���ֽ���
	bool Line2Stream(char *stream, Grid::Line &line, char state, int &size, unsigned int space);
	//���ֽ�����ȡ������
	bool Stream2Line(Grid::Line &line, char &state, int &size, char *stream, unsigned int space);


//////////////////////////////////////////////////////////////////////////
//���Է���
public:
	Grid::Result::Result ReadPoint(Grid::Point &point);
	Grid::Result::Result ReadLine(Grid::Line &line);
private:
	Grid::Result::Result ReadIndex(mdk::File &ideFile, bigint id, bigint &idxPos, bigint &dataPos);

private:
	std::string	m_dataDir;//����Ŀ¼
	mdk::File m_pointIdx;	//���������ļ�
	mdk::File m_pointData[Grid::Limit::maxDBShardCount];	//���������ļ�
	mdk::File m_lineIdx;	//�������ļ�
	mdk::File m_lineData[Grid::Limit::maxDBShardCount];	//�������ļ�
	GridStore *m_pStore;

	char	*m_stream;//���ݶ���ת����д����̣��ʹӴ��̶�ȡ��ʱ��洢����(����)
	int		m_streamSpace;//�ռ��С
	char	*m_idxStream;//��������ת����д����̣��ʹӴ��̶�ȡ��ʱ��洢����(����)
	int		m_idxStreamSpace;//���������ռ��С

};

#endif //DISKIO_H
