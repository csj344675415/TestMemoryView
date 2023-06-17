#pragma once

#include "InfoViewBase.h"

typedef struct QTHREAD_INFO
{
	unsigned int threadId;					//�߳�id
	unsigned int threadPri;					//���ȼ�		
	unsigned long long threadStartAddr;		//��ʼ��ַ
};

class InfoViewThread : public InfoViewBase
{
	Q_OBJECT

public:
	InfoViewThread(QWidget *parent = Q_NULLPTR);
	~InfoViewThread();

	private slots:
	void slot_pauseThread();
	void slot_recoverThread();
	void slot_pauseThreads();
	void slot_recoverThreads();
	void slot_quitThread();


protected: //�ܱ���
	virtual void initMenu();
	virtual void enmuInfo(unsigned int pid);
	virtual void updateView(unsigned int pid = 0);

	QVector<QTHREAD_INFO> _vecInfo;

private:

};
