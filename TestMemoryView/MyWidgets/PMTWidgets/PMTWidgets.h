#pragma once

#include <QWidget>
#include <QIcon>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma warning(disable:4091 4311)

//class QIcon;
class QTableView;
class QMenu;
class QSortFilterProxyModel;

class MyAbstractTableModel;
class MyHeaderView;

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

typedef struct QMODULE_INFO
{
	QString moduleName;					//ģ����
	QString modulePath;					//ģ��·��
	unsigned long long moduleBaseAddr;	//ģ����ʼ��ַ
	unsigned long long moduleSize;		//ģ���С
	unsigned int gcount;				//ȫ�����ü���
	unsigned int count;					//���ü���
};

typedef struct QTHREAD_INFO
{
	unsigned int threadId;					//�߳�id
	unsigned int threadPri;					//���ȼ�		
	unsigned long long threadStartAddr;		//��ʼ��ַ
};

class PMTWidgets : public QWidget
{
	Q_OBJECT

public:
	
	typedef enum PMTW_TYPE {
		eProcess = 0,	//ö�ٽ���
		eMoudle,		//ö��ģ��
		eThread,		//ö���߳�
	};

	PMTWidgets(QWidget *parent = Q_NULLPTR);
	PMTWidgets(PMTW_TYPE nViewType = PMTW_TYPE::eProcess, QWidget *parent = Q_NULLPTR);
	~PMTWidgets();

private slots:
	void slot_customContextMenuRequested(const QPoint &pos);
	//ͨ��
	void slot_clickedView(const QModelIndex &index);	//���
	void slot_doubleView(const QModelIndex &index);		//˫��
	void slot_update();
	void slot_openDir();
	void slot_filter();
	void slot_copy();

	//����
	void slot_select();
	void slot_pauseProcess();
	void slot_recoverProcess();
	void slot_quitProcess();
	void slot_simplePro();		//��
	void slot_ordinaryPro();	//��ͨ
	void slot_detailedPro();	//��ϸ

	//ģ��
	void slot_injectDll();
	void slot_unInjectDll();

	//�߳�
	void slot_pauseThread();
	void slot_recoverThread();
	void slot_pauseThreads();
	void slot_recoverThreads();
	void slot_quitThread();

	//����
	void slot_testButton();	//���԰�ť

public:
	signals:
	void selectProcess(QPROCESS_INFO pi);	//ѡ�����
	void clickedView_p(QPROCESS_INFO pi);	//��������б��ź�
	void doubleView_p(QPROCESS_INFO pi);	//˫�������б��ź�
	void clickedView_m(QMODULE_INFO pi);	//���ģ���б��ź�
	void doubleView_m(QMODULE_INFO pi);		//˫��ģ���б��ź�
	void viewUpdate();						//��ͼ����




public:
	//���ýӿ�
	void showProcessView();								//��ʾ�����б�
	void showModuleView(unsigned int pid);				//��ʾģ���б�
	void showThreadView(unsigned int pid);				//��ʾ�߳��б�
	void setProcessListStyle(int style = 0);			//0��Լ	1��ͨ	2��ϸ
	int getViewType() { return __nViewType; }			//��ȡ��ǰ��ʾ����ͼ����
	void setResize(unsigned int w, unsigned int h);		//������ͼ��С
	void setViewType(PMTW_TYPE nViewType) { __nViewType = nViewType; }	//������ͼ��ʾ���ͣ�0���̡�1ģ�顢2�߳�
	int getRowCount();									//��ȡ��ǰ��ͼ���͵���ʾ������
	QString getData(int row, int col);					//��ȡĳ��Ԫ������
	void toRow(int row);								//����ָ���в�ѡ��



private:
	QSortFilterProxyModel *__pProxyModel = nullptr;
	MyAbstractTableModel *__pModel = nullptr;
	MyHeaderView *__pMyHeaderView = nullptr;
	QMenu *__pMenu_Pro = nullptr;
	QMenu *__pMenu_Mod = nullptr;
	QMenu *__pMenu_Thr = nullptr;
	QTableView *__pView;

	QVector<QPROCESS_INFO> __vecPro;
	QVector<QMODULE_INFO> __vecMod;
	QVector<QTHREAD_INFO> __vecThr;
	
	
	int __nViewType = 0;	//��ͼ���ͣ����̡�ģ�顢�߳�
	int __nCurRow = 0;		//��ͼ��ǰѡ����
	int __nCurCol = 0;		//��ͼ��ǰѡ����

	int __nW = 250;
	int __nH = 300;
	
	void InitPMT();
	void InitTableView();
	bool EnablePrivilege();
	void createMenu();

	bool __bFilter = true;	//����
	unsigned int __pid = 0;	

	//--------
	int __nStyle = 0;
	QString __exeName;
	void enmuProcess();
	void showProcessList(int style = 0);
	void showProList();
	

	//--------
	QString __moduleName;
	QString __LastFilePath;		//�ϴ��ļ��������򿪵�·��
	void enmuModule(DWORD pid);
	void showModList();
	bool Inject_CreateRemoteThread(const char* pszDllFile, DWORD dwProcessId);
	bool UnInjectDll(const char* ptszDllFile, DWORD dwProcessId);


	//--------
	void enmuThread(DWORD pid);
	void showThrList();
};
