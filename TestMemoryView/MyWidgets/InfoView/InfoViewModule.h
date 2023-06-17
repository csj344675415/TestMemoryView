#pragma once

#include "InfoViewBase.h"

typedef struct QMODULE_INFO
{
	QString moduleName;					//ģ����
	QString modulePath;					//ģ��·��
	unsigned long long moduleBaseAddr;	//ģ����ʼ��ַ
	unsigned long long moduleSize;		//ģ���С
	unsigned int gcount;				//ȫ�����ü���
	unsigned int count;					//���ü���
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
	//-------------- �ⲿ�ӿ� -------------------//
	void toRow(QString text, int col);


protected: //�ܱ���
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
