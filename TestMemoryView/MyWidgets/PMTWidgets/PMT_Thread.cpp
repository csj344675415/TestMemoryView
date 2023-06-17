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
	IN HANDLE ThreadHandle,            //�߳̾��
	IN ULONG  ThreadInformationClass,  //�߳���Ϣö���࣬Ҫ��ȡ���߳���Ϣ   ö���ࣨ_THREADINFOCLASS��
	OUT PVOID ThreadInformation,       //��ȡ����Ϣ����Ļ�������ַ
	IN ULONG  ThreadInformationLength, //��������С
	OUT PULONG ReturnLength);          //ʵ��д���С
typedef enum _THREAD_INFO_CLASS
{
	ThreadBasicInformation,
	ThreadTimes,         //�߳�ʱ��
	ThreadPriority,      //�߳����ȼ�
	ThreadBasePriority,
	ThreadAffinityMask,
	ThreadImpersonationToken,
	ThreadDescriptorTableEntry,
	ThreadEnableAlignmentFaultFixup,
	ThreadEventPair_Reusable,
	ThreadQuerySetWin32StartAddress, //�߳���ʼ��ַ
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

	//��ȡNtQueryInformationThread������ַ
	HMODULE hNtdll = LoadLibraryW(L"ntdll.dll");
	NTQUERYINFORMATIONTHREAD NtQueryInformationThread = NULL;
	NtQueryInformationThread = (NTQUERYINFORMATIONTHREAD)GetProcAddress(hNtdll, "NtQueryInformationThread");

	//��ȡϵͳ�����߳̿���
	HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, NULL);
	//�жϵ�һ���߳̿����Ƿ���Ч
	if (Thread32First(Snapshot, &te32)) {
		__vecThr.clear();
		do {
			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, te32.th32ThreadID);
			//GetProcessIdOfThread()ͨ���߳̾����ȡ��Ӧ�Ľ���pid
			//�жϸ��߳��Ƿ�����ָ���Ľ��̣��粻���ж���Ϊ��ӡ��ǰϵͳ�����߳���Ϣ
			if (pid == GetProcessIdOfThread(hThread)) {

				//��ȡ�߳���Ϣ���˴�Ϊ��ȡ�̵߳���ʼ��ַ��Ϣ
				NtQueryInformationThread(hThread, ThreadQuerySetWin32StartAddress, &dwStaAddr, sizeof(dwStaAddr), &dwReturnLength);

				QTHREAD_INFO ti;
				ti.threadId = te32.th32ThreadID;	//�߳�ID
				ti.threadPri = te32.tpBasePri;		//�߳����ȼ�
				ti.threadStartAddr = dwStaAddr;		//�߳���ʼ��ַ
				__vecThr.append(ti);
			}
			CloseHandle(hThread);
		} while (Thread32Next(Snapshot, &te32)); //�ж���һ���߳̿����Ƿ���Ч����Ч�����ѭ��
	}
	//��β����
	CloseHandle(Snapshot); //�رտ��վ��
}

void PMTWidgets::showThrList()
{
	__pModel->clear();

	//���ñ�ͷ����
	__pModel->setHeaderData(0, u8"�߳�ID");
	__pModel->setHeaderData(1, u8"���ȼ�");
	__pModel->setHeaderData(2, u8"��ʼ��ַ");

	int nTo = -1;
	int nCount = 0;
	for (int i = 0; i < __vecThr.size(); ++i)
	{
		//���ò���ʾ������,���������Ҫ������������	 
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
		__pView->scrollTo(__pProxyModel->index(nTo, 0), QAbstractItemView::PositionAtCenter);	//����ָ������
	}
	else {
		__pView->scrollToBottom();
	}

	__pView->setColumnWidth(0, 80);
	__pView->setColumnWidth(1, 60);
	//__pView->setColumnWidth(2, 60);

	setWindowTitle(u8"�̣߳�" + QString::number(nCount));

	//��������ź�
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

		QString strMsg = u8"�Ƿ�Ҫ�˳����߳�";
		int click = QMessageBox::warning(NULL, u8"����", strMsg, QMessageBox::Yes | QMessageBox::No);
		if (click == QMessageBox::Yes) {
			if (::TerminateThread(handle, 0)) {
				__pModel->removeRow(__nCurRow);
			}
		}
		CloseHandle(handle);
	}
}