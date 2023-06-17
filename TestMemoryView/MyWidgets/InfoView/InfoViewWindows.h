#pragma once

//#include "InfoViewBase.h"
#include <QTreeView>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma warning(disable:4091 4311)

class QStandardItemModel;

typedef struct QWINDOW_INFO
{
	unsigned long long hwnd;	//���ھ��
	unsigned long long hwndParent;	//�����ھ��
	unsigned int threadId;		//�߳�id	
	QString title;				//���ڱ���
	QString className;			//��������
	QRect rect;					//���ڴ�С
};

class InfoViewWindows : public QTreeView
{
	Q_OBJECT

public:
	InfoViewWindows(QWidget *parent = Q_NULLPTR);
	~InfoViewWindows();

	private slots:

public:
	void showInfoTree(unsigned int pid = 0);
	QVector<QWINDOW_INFO> _vecInfo;

protected: //�ܱ���
	//virtual void initMenu();
	QStandardItemModel* _pModel;

private:
	virtual void initTree();
	virtual void enmuInfo(unsigned int pid);
	virtual void updateView(unsigned int pid);
};

