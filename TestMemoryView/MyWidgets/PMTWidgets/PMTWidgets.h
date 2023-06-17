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
	QString moduleName;					//模块名
	QString modulePath;					//模块路径
	unsigned long long moduleBaseAddr;	//模块起始地址
	unsigned long long moduleSize;		//模块大小
	unsigned int gcount;				//全局引用计数
	unsigned int count;					//引用计数
};

typedef struct QTHREAD_INFO
{
	unsigned int threadId;					//线程id
	unsigned int threadPri;					//优先级		
	unsigned long long threadStartAddr;		//起始地址
};

class PMTWidgets : public QWidget
{
	Q_OBJECT

public:
	
	typedef enum PMTW_TYPE {
		eProcess = 0,	//枚举进程
		eMoudle,		//枚举模块
		eThread,		//枚举线程
	};

	PMTWidgets(QWidget *parent = Q_NULLPTR);
	PMTWidgets(PMTW_TYPE nViewType = PMTW_TYPE::eProcess, QWidget *parent = Q_NULLPTR);
	~PMTWidgets();

private slots:
	void slot_customContextMenuRequested(const QPoint &pos);
	//通用
	void slot_clickedView(const QModelIndex &index);	//点击
	void slot_doubleView(const QModelIndex &index);		//双击
	void slot_update();
	void slot_openDir();
	void slot_filter();
	void slot_copy();

	//进程
	void slot_select();
	void slot_pauseProcess();
	void slot_recoverProcess();
	void slot_quitProcess();
	void slot_simplePro();		//简单
	void slot_ordinaryPro();	//普通
	void slot_detailedPro();	//详细

	//模块
	void slot_injectDll();
	void slot_unInjectDll();

	//线程
	void slot_pauseThread();
	void slot_recoverThread();
	void slot_pauseThreads();
	void slot_recoverThreads();
	void slot_quitThread();

	//测试
	void slot_testButton();	//测试按钮

public:
	signals:
	void selectProcess(QPROCESS_INFO pi);	//选择进程
	void clickedView_p(QPROCESS_INFO pi);	//点击进程列表信号
	void doubleView_p(QPROCESS_INFO pi);	//双击进程列表信号
	void clickedView_m(QMODULE_INFO pi);	//点击模块列表信号
	void doubleView_m(QMODULE_INFO pi);		//双击模块列表信号
	void viewUpdate();						//视图更新




public:
	//调用接口
	void showProcessView();								//显示进程列表
	void showModuleView(unsigned int pid);				//显示模块列表
	void showThreadView(unsigned int pid);				//显示线程列表
	void setProcessListStyle(int style = 0);			//0简约	1普通	2详细
	int getViewType() { return __nViewType; }			//获取当前显示的视图类型
	void setResize(unsigned int w, unsigned int h);		//设置视图大小
	void setViewType(PMTW_TYPE nViewType) { __nViewType = nViewType; }	//设置视图显示类型：0进程、1模块、2线程
	int getRowCount();									//获取当前视图类型的显示的行数
	QString getData(int row, int col);					//获取某单元格数据
	void toRow(int row);								//跳到指定行并选中



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
	
	
	int __nViewType = 0;	//视图类型：进程、模块、线程
	int __nCurRow = 0;		//视图当前选中行
	int __nCurCol = 0;		//视图当前选中列

	int __nW = 250;
	int __nH = 300;
	
	void InitPMT();
	void InitTableView();
	bool EnablePrivilege();
	void createMenu();

	bool __bFilter = true;	//过滤
	unsigned int __pid = 0;	

	//--------
	int __nStyle = 0;
	QString __exeName;
	void enmuProcess();
	void showProcessList(int style = 0);
	void showProList();
	

	//--------
	QString __moduleName;
	QString __LastFilePath;		//上次文件管理器打开的路径
	void enmuModule(DWORD pid);
	void showModList();
	bool Inject_CreateRemoteThread(const char* pszDllFile, DWORD dwProcessId);
	bool UnInjectDll(const char* ptszDllFile, DWORD dwProcessId);


	//--------
	void enmuThread(DWORD pid);
	void showThrList();
};
