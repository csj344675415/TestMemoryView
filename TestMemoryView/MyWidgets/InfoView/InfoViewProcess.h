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
	void slot_simplePro();		//��
	void slot_ordinaryPro();	//��ͨ
	void slot_detailedPro();	//��ϸ

signals:
	void signal_selectProcess(const QModelIndex & index);
	//void signal_clickedView(const QModelIndex & index);	//��������б��ź�
	//void signal_doubleView(const QModelIndex & index);	//˫�������б��ź�

public:
	QPROCESS_INFO getRowData(const QModelIndex & index);

protected: //�ܱ���
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
