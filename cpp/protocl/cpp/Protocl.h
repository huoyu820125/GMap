#ifndef PROTOCL_H
#define PROTOCL_H

#define MAX_MSG_SIZE 10240	//������󳤶�
#define BYTE_ORDER	net::smallOrder	//�ֽ���

//ģ�飨���񣩶���
namespace Moudle
{
	enum Moudle
	{
		all = 0,				//����ģ��
		gsrc = 1,		//��Ⱥ����ģ��
		gmap = 2,			//ͼ���ݿ�
	};
}

//����Id
namespace MsgId
{
	enum MsgId
	{
		//��Ⱥ����ģ��
		getCluster = 1001,//ȡ��Ⱥ��Ϣ
		getClientId = 1002,//ȡΨһclient id

		//ͼ���ݿ�������
		newId = 1001,//����һ��Ψһid
		setPoint = 1002,//���ö���
		addLineForPoint = 1003,//��������ӱ�
		delLineForPoint = 1004,//ɾ������ı�
		getPoint = 1005,//ȡ�ö���
		addLine = 1006,//������
		delLine = 1007,//ɾ����
		getLine = 1008,//ȡ�ñ�
		delPoint = 1009,//ɾ������
		setMoveOpt = 1010,//����move������move2 move1 filterPoint����
		move = 1011,//�༶�ƶ�
		points = 1012,//�ҵ��Ķ���
		moveMids = 1013,//move�����е�ǰ�ڵ������ݵĶ���id
		//��������
		setPoints = 1014,//�������ö���
		linkPoints = 1015,//�������Ӷ���
		delPoints = 1016,//����ɾ������
	};
}

namespace ResultCode
{
	enum ResultCode
	{
		success = 0, //�ɹ�
		paramError = 1,//��������
		netError = 2,//�������
		msgError = 3,//����˱��Ĵ����޷���������Ԥ�ڵı���
		refuse = 4,//�ܾ�
	};
};

#endif //PROTOCL_H