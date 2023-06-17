#pragma once

//#include "InfoViewBase.h"
#include <QTreeView>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma warning(disable:4091 4311)

class QStandardItemModel;

typedef struct QWINDOW_INFO
{
	unsigned long long hwnd;	//窗口句柄
	unsigned long long hwndParent;	//父窗口句柄
	unsigned int threadId;		//线程id	
	QString title;				//窗口标题
	QString className;			//窗口类名
	QRect rect;					//窗口大小
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

protected: //受保护
	//virtual void initMenu();
	QStandardItemModel* _pModel;

private:
	virtual void initTree();
	virtual void enmuInfo(unsigned int pid);
	virtual void updateView(unsigned int pid);
};

