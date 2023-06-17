#include "InfoViewThread.h"
#include <QMenu>
#include <QMessageBox>

#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib,"Psapi.lib")

InfoViewThread::InfoViewThread(QWidget *parent)
	: InfoViewBase(parent)
{
	resize(600, 305);
	initView();
}

InfoViewThread::~InfoViewThread()
{

}


void InfoViewThread::initMenu()
{
	_pMenu = new QMenu(this);
	_pMenu->addAction(u8"刷新", this, SLOT(slot_update()));
	_pMenu->addSeparator();
	_pMenu->addAction(u8"复制表格数据", this, SLOT(slot_copy()));
	_pMenu->addSeparator();
	_pMenu->addAction(u8"暂停所有线程", this, SLOT(slot_pauseThreads()));
	_pMenu->addAction(u8"运行所有线程", this, SLOT(slot_recoverThreads()));
	_pMenu->addAction(u8"暂停线程", this, SLOT(slot_pauseThread()));
	_pMenu->addAction(u8"运行线程", this, SLOT(slot_recoverThread()));
	_pMenu->addAction(u8"退出线程", this, SLOT(slot_quitThread()));
}


typedef LONG NTSTATUS;
typedef NTSTATUS(WINAPI *NTQUERYINFORMATIONTHREAD)(
	IN HANDLE ThreadHandle,            //线程句柄
	IN ULONG  ThreadInformationClass,  //线程信息枚举类，要获取的线程信息   枚举类（_THREADINFOCLASS）
	OUT PVOID ThreadInformation,       //获取的信息存入的缓冲区地址
	IN ULONG  ThreadInformationLength, //缓冲区大小
	OUT PULONG ReturnLength);          //实际写入大小
typedef enum _THREAD_INFO_CLASS
{
	ThreadBasicInformation,
	ThreadTimes,         //线程时间
	ThreadPriority,      //线程优先级
	ThreadBasePriority,
	ThreadAffinityMask,
	ThreadImpersonationToken,
	ThreadDescriptorTableEntry,
	ThreadEnableAlignmentFaultFixup,
	ThreadEventPair_Reusable,
	ThreadQuerySetWin32StartAddress, //线程起始地址
	ThreadZeroTlsCell,
	ThreadPerformanceCount,
	ThreadAmILastThread,
	ThreadIdealProcessor,
	ThreadPriorityBoost,
	ThreadSetTlsArrayAddress,   // Obsolete
	ThreadIsIoPending,
	ThreadHideFromDebugger,
	ThreadBreakOnTermination,
	ThreadSwitchLegacyState,
	ThreadIsTerminated,
	ThreadLastSystemCall,
	ThreadIoPriority,
	ThreadCycleTime,
	ThreadPagePriority,
	ThreadActualBasePriority,
	ThreadTebInformation,
	ThreadCSwitchMon,          // Obsolete
	ThreadCSwitchPmu,
	ThreadWow64Context,
	ThreadGroupInformation,
	ThreadUmsInformation,      // UMS
	ThreadCounterProfiling,
	ThreadIdealProcessorEx,
	MaxThreadInfoClass
} THREAD_INFO_CLASS, *PTHREAD_INFO_CLASS;



void InfoViewThread::enmuInfo(unsigned int pid)
{
	if (pid == 0) return;
	bool is64 = is64Process(pid);
	DWORD_PTR dwStaAddr = NULL;
	DWORD dwReturnLength = NULL;
	HANDLE hThread = NULL;
	THREADENTRY32 te32;
	te32.dwSize = sizeof(te32);

	//获取NtQueryInformationThread函数地址
	HMODULE hNtdll = LoadLibraryW(L"ntdll.dll");
	NTQUERYINFORMATIONTHREAD NtQueryInformationThread = NULL;
	NtQueryInformationThread = (NTQUERYINFORMATIONTHREAD)GetProcAddress(hNtdll, "NtQueryInformationThread");

	//获取系统所有线程快照
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	//判断第一个线程快照是否有效
	if (Thread32First(Snapshot, &te32)) {
		_vecInfo.clear();
		do {
			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
			//GetProcessIdOfThread()通过线程句柄获取对应的进程pid
			//判断该线程是否属于指定的进程，如不做判断则为打印当前系统所有线程信息
			if (pid == GetProcessIdOfThread(hThread)) {

				//获取线程信息，此处为获取线程的起始地址信息
				NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &dwStaAddr, sizeof(dwStaAddr), &dwReturnLength);

				QTHREAD_INFO ti;
				ti.threadId = te32.th32ThreadID;	//线程ID
				ti.threadPri = te32.tpBasePri;		//线程优先级
				ti.threadStartAddr = dwStaAddr;		//线程起始地址
				_vecInfo.append(ti);
			}
			CloseHandle(hThread);
		} while (Thread32Next(Snapshot, &te32)); //判断下一个线程快照是否有效，有效则进入循环
	}
	//收尾工作
	CloseHandle(Snapshot); //关闭快照句柄
}

void InfoViewThread::updateView(unsigned int pid)
{
	_pModel->clear();

	// 设置表头内容
	QStringList headerData;
	headerData
		<< QStringLiteral("线程ID")
		<< QStringLiteral("优先级")
		<< QStringLiteral("起始地址");
	_pModel->setHorizontalHeaderLabels(headerData);

	

	//设置Item内容
	int nTo = -1;
	int nCount = 0;
	for (int i = 0; i < _vecInfo.size(); ++i)
	{
		// 设置表格数据
		QList<QStandardItem*> items;
		items
			<< new QStandardItem(QString::number(_vecInfo[i].threadId).toUpper())
			<< new QStandardItem(QString::number(_vecInfo[i].threadPri).toUpper())
			<< new QStandardItem(QString::number(_vecInfo[i].threadStartAddr, 16).toUpper());

		nCount++;
		_pModel->appendRow(items);
	}

	this->setColumnWidth(0, 80);
	this->setColumnWidth(1, 60);
	//__pView->setColumnWidth(2, 60);

	this->setWindowTitle(u8"线程：" + QString::number(nCount));
}



void InfoViewThread::slot_pauseThread()
{
	QModelIndex curIndex = this->currentIndex();
	if (curIndex.isValid()) {
		HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, _vecInfo[curIndex.row()].threadId);
		SuspendThread(handle);
		CloseHandle(handle);
		for (int i = 0; i < _pModel->columnCount(); ++i) {
			auto item = _pModel->item(curIndex.row(), i);
			if (item) {
				item->setForeground(QColor(Qt::red));
			}
		}
	}
}
void InfoViewThread::slot_recoverThread()
{
	QModelIndex curIndex = this->currentIndex();
	if (curIndex.isValid()) {
		HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, _vecInfo[curIndex.row()].threadId);
		ResumeThread(handle);
		CloseHandle(handle);
		for (int i = 0; i < _pModel->columnCount(); ++i) {
			auto item = _pModel->item(curIndex.row(), i);
			if (item) {
				item->setForeground(QColor(nullptr));
			}
		}
	}
}

void InfoViewThread::slot_pauseThreads()
{
	for (int j = 0; j < _pModel->rowCount(); ++j) {
		HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, _vecInfo[j].threadId);
		SuspendThread(handle);
		CloseHandle(handle);
		for (int i = 0; i < _pModel->columnCount(); ++i) {
			auto item = _pModel->item(j, i);
			if (item) {
				item->setForeground(QColor(Qt::red));
			}
		}
	}
}
void InfoViewThread::slot_recoverThreads()
{
	for (int j = 0; j < _pModel->rowCount(); ++j) {
		HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, _vecInfo[j].threadId);
		ResumeThread(handle);
		CloseHandle(handle);
		for (int i = 0; i < _pModel->columnCount(); ++i) {
			auto item = _pModel->item(j, i);
			if (item) {
				item->setForeground(QColor(nullptr));
			}
		}
	}
}

void InfoViewThread::slot_quitThread()
{
	QModelIndex curIndex = this->currentIndex();
	if (curIndex.isValid()) {
		HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, _vecInfo[curIndex.row()].threadId);

		QString strMsg = u8"是否要退出此线程";
		int click = QMessageBox::warning(NULL, u8"警告", strMsg, QMessageBox::Yes | QMessageBox::No);
		if (click == QMessageBox::Yes) {
			if (::TerminateThread(handle, 0)) {
				_pModel->removeRow(curIndex.row());
			}
		}
		CloseHandle(handle);
	}
}
