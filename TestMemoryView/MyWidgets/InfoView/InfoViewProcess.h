#pragma once

#include "InfoViewBase.h"

typedef struct QPROCESS_INFO
{
	QIcon icon;
	QString exeName;
	QString exePath;
	QString title;
	bool is64bit;
	unsigned int pid;
	unsigned tid;
	unsigned threadCount;
	HWND hwnd;
};

class InfoViewProcess : public InfoViewBase
{
	Q_OBJECT

public:
	InfoViewProcess(QWidget *parent = Q_NULLPTR);
	~InfoViewProcess();

	private slots:
	virtual void slot_clickedView(const QModelIndex & index);
	//virtual void slot_doubleView(const QModelIndex & index);

	void slot_select();
	void slot_openDir();
	void slot_pauseProcess();
	void slot_recoverProcess();
	void slot_quitProcess();
	void slot_simplePro();		//简单
	void slot_ordinaryPro();	//普通
	void slot_detailedPro();	//详细

signals:
	void signal_selectProcess(const QModelIndex & index);
	//void signal_clickedView(const QModelIndex & index);	//点击进程列表信号
	//void signal_doubleView(const QModelIndex & index);	//双击进程列表信号

public:
	QPROCESS_INFO getRowData(const QModelIndex & index);

protected: //受保护
	virtual void initMenu();
	virtual void enmuInfo(unsigned int pid = 0);
	virtual void updateView(unsigned int pid = 0);
	virtual void readSettings();
	virtual void writeSettings();

	QVector<QPROCESS_INFO> _vecInfo;
	int _nStyle;

private:
	
	HWND pid2Hwnd(DWORD pid);
	QString __exeName;
};
