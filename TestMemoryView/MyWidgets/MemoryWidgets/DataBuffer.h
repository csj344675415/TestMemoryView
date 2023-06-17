#pragma once

#include <QObject>
#include <QVector>
#include <QHash>
#include <QColor>

#include <thread>

#pragma warning(disable:4091)
#pragma warning(disable:4996)

class QTimerEvent;

#ifndef STRUCT_DataBuffer
typedef struct QMODULE_INFO_T
{
	unsigned long long moduleBaseAddr;	//ģ����ʼ��ַ
	unsigned long long moduleSize;		//ģ���С
	QString moduleName;					//ģ����
	QString modulePath;					//ģ��·��
	unsigned int gcount;				//ȫ�����ü���
	unsigned int count;					//���ü���
};

typedef struct RENDER_DATA_INFO {
	int ms;				//Ⱦɫʣ��ʱ��
	unsigned char diaphaneity;	//͸����
};

typedef struct MEM_INFO_T
{
	unsigned long long firstAddr;		//��ǰҳ�Ľ�����ַ
	unsigned long long allocationBase;	//�����ڴ�ҳ��Ӧ��ģ���ַ
	unsigned long long beginAddr;		//��ǰҳ�Ŀ�ʼ��ַ
	unsigned long long endAddr;			//��ǰҳ�Ľ�����ַ
	unsigned int size;					//��ǰҳ�Ĵ�С
	unsigned int state;					//�ڴ�״̬
	unsigned int state2;				//�ڴ�״̬
	unsigned int type;					//�ڴ�����
	unsigned int protect;				//��ǰҳ�ı�������
	QString strProtect;					//ҳ���������ı�
	QColor color;
	QColor color2;
	QString moduleName;
};
#define STRUCT_DataBuffer
#endif

class DataBuffer : public QObject
{
	Q_OBJECT

public:
	DataBuffer(QObject *parent);
	~DataBuffer();

protected:
	void timerEvent(QTimerEvent *event);


	signals:
	void signal_dataChange();	//���ݱ仯�������ź�
	void signal_update();		//�ڶ�ʱ���﷢������źţ�֪ͨView����
public:
	MEM_INFO_T setData(unsigned int pid, unsigned long long addr = -1);
	MEM_INFO_T changeAddr(unsigned long long addr = -1);
	void changeTime(int ms);
	void updateStop();	//ֹͣ����ˢ��
	void updateStart();	//��ʼ����ˢ��
	unsigned char* data() { return _buffer; }
	QHash<unsigned long long, RENDER_DATA_INFO>* getHashTable() { return &_hashAddrTable; }

protected:
	void getModuleInfo(unsigned int pid);

	unsigned int _pid;
	std::thread* _t = nullptr;	//�߳�ָ��
	bool _bStrat_t = false;	//�߳̿��ر�־
	void _tUpdateData();		//�߳�ˢ�º���
	int _nTimerID2 = -1;		//����䶯���ݵĶ�ʱ�����䶯���ݵı���ɫ�𽥱䵭
	int _interval = 330;		//��ʱ�������ļ��ʱ��
	unsigned int _dataOffset = 0;
	unsigned int _dataSize = 0;
	QVector<QMODULE_INFO_T> _vecModule;

	unsigned char _buffer[1024];
	unsigned long long _firstAddr = -1;
	int _maxBytes = 1024;
	
	MEM_INFO_T _strMI;

	int _ms = 1000;				//�䶯���ݵĸ���ʱ���ʱ��
	QHash<unsigned long long, RENDER_DATA_INFO> _hashAddrTable;	//���ݱ䶯ʱ��ҪȾɫ�����ݵ�ַ��ϣ��
};
