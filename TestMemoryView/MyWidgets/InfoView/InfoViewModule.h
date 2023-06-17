#pragma once

#include "InfoViewBase.h"

typedef struct QMODULE_INFO
{
	QString moduleName;					//模块名
	QString modulePath;					//模块路径
	unsigned long long moduleBaseAddr;	//模块起始地址
	unsigned long long moduleSize;		//模块大小
	unsigned int gcount;				//全局引用计数
	unsigned int count;					//引用计数
};

class InfoViewModule : public InfoViewBase
{
	Q_OBJECT

public:
	InfoViewModule(QWidget *parent = Q_NULLPTR);
	~InfoViewModule();


	private slots:
	virtual void slot_clickedView(const QModelIndex & index);

	void slot_openDir();
	void slot_injectDll();
	void slot_unInjectDll();

public:
	//-------------- 外部接口 -------------------//
	void toRow(QString text, int col);


protected: //受保护
	virtual void initMenu();
	virtual void enmuInfo(unsigned int pid);
	virtual void updateView(unsigned int pid = 0);
	virtual void readSettings();
	virtual void writeSettings();
	bool Inject_CreateRemoteThread(const char* pszDllFile, DWORD dwProcessId);
	bool UnInjectDll(const char* ptszDllFile, DWORD dwProcessId);

	QVector<QMODULE_INFO> _vecInfo;

private:
	QString __moduleName;
	QString __LastFilePath;
};
