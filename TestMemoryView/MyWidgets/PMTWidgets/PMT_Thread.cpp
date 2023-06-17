#include "PMTWidgets.h"
#include <QMessageBox>
#include <QTableView>

#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib,"Psapi.lib")

#include "MyAbstractTableModel.h"
#include "MyHeaderView.h"

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

extern bool is64Process(unsigned int dwProcessID);

void PMTWidgets::enmuThread(DWORD pid)
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
		__vecThr.clear();
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
				__vecThr.append(ti);
			}
			CloseHandle(hThread);
		} while (Thread32Next(Snapshot, &te32)); //判断下一个线程快照是否有效，有效则进入循环
	}
	//收尾工作
	CloseHandle(Snapshot); //关闭快照句柄
}

void PMTWidgets::showThrList()
{
	__pModel->clear();

	//设置表头内容
	__pModel->setHeaderData(0, u8"线程ID");
	__pModel->setHeaderData(1, u8"优先级");
	__pModel->setHeaderData(2, u8"起始地址");

	int nTo = -1;
	int nCount = 0;
	for (int i = 0; i < __vecThr.size(); ++i)
	{
		//设置不显示的数据,列随意填，不要超过列数就行	 
		__pModel->setUserData(nCount, 0, i);
		__pModel->setItemData(nCount, 0, QString::number(__vecThr[i].threadId).toUpper());
		__pModel->setItemData(nCount, 1, QString::number(__vecThr[i].threadPri).toUpper());
		__pModel->setItemData(nCount, 2, QString::number(__vecThr[i].threadStartAddr,16).toUpper());
		nCount++;
	}

	__pModel->updateData();

	if (nTo != -1) {
		__pView->setFocus();
		__pView->selectRow(nTo);
		__pView->scrollTo(__pProxyModel->index(nTo, 0), QAbstractItemView::PositionAtCenter);	//滚到指定索引
	}
	else {
		__pView->scrollToBottom();
	}

	__pView->setColumnWidth(0, 80);
	__pView->setColumnWidth(1, 60);
	//__pView->setColumnWidth(2, 60);

	setWindowTitle(u8"线程：" + QString::number(nCount));

	//发射更新信号
	emit viewUpdate();
}

void PMTWidgets::slot_pauseThread()
{
	QModelIndex modelIndex = __pModel->getModelIndex(__nCurRow, 0);
	if (modelIndex.isValid()) {
		int n = modelIndex.data(Qt::UserRole + 1).toInt();
		HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, __vecThr[n].threadId);
		SuspendThread(handle);
		CloseHandle(handle);
		for (int i = 0; i < __pModel->getColumns(); ++i) {
			__pModel->setTextColor(__nCurRow, i, Qt::red);
		}

	}
}
void PMTWidgets::slot_recoverThread()
{
	QModelIndex modelIndex = __pModel->getModelIndex(__nCurRow, 0);
	if (modelIndex.isValid()) {
		int n = modelIndex.data(Qt::UserRole + 1).toInt();
		HANDLE handle = OpenThread(THREAD_ALL_ACCESS, FALSE, __vecThr[n].threadId);
		ResumeThread(handle);
		CloseHandle(handle);
		for (int i = 0; i < __pModel->getColumns(); ++i) {
			__pModel->setTextColor(__nCurRow, i, nullptr);
		}
	}
}

void PMTWidgets::slot_pauseThreads()
{
	for (int j = 0; j < getRowCount(); ++j) {
		QModelIndex modelIndex = __pModel->getModelIndex(j, 0);
		if (modelIndex.isValid()) {
			int n = modelIndex.data(Qt::UserRole + 1).toInt();
			HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, __vecThr[n].threadId);
			SuspendThread(handle);
			CloseHandle(handle);
			for (int i = 0; i < __pModel->getColumns(); ++i) {
				__pModel->setTextColor(j, i, Qt::red);
			}
		}
	}	
}
void PMTWidgets::slot_recoverThreads()
{
	for (int j = 0; j < getRowCount(); ++j) {
		QModelIndex modelIndex = __pModel->getModelIndex(j, 0);
		if (modelIndex.isValid()) {
			int n = modelIndex.data(Qt::UserRole + 1).toInt();
			HANDLE handle = OpenThread(THREAD_ALL_ACCESS, FALSE, __vecThr[n].threadId);
			ResumeThread(handle);
			CloseHandle(handle);
			for (int i = 0; i < __pModel->getColumns(); ++i) {
				__pModel->setTextColor(j, i, nullptr);
			}
		}
	}
}

void PMTWidgets::slot_quitThread()
{
	QModelIndex modelIndex = __pModel->getModelIndex(__nCurRow, 0);
	if (modelIndex.isValid()) {
		int n = modelIndex.data(Qt::UserRole + 1).toInt();
		HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, __vecThr[n].threadId);

		QString strMsg = u8"是否要退出此线程";
		int click = QMessageBox::warning(NULL, u8"警告", strMsg, QMessageBox::Yes | QMessageBox::No);
		if (click == QMessageBox::Yes) {
			if (::TerminateThread(handle, 0)) {
				__pModel->removeRow(__nCurRow);
			}
		}
		CloseHandle(handle);
	}
}