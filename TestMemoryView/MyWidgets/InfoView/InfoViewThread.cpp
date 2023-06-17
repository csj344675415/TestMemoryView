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
	_pMenu->addAction(u8"ˢ��", this, SLOT(slot_update()));
	_pMenu->addSeparator();
	_pMenu->addAction(u8"���Ʊ������", this, SLOT(slot_copy()));
	_pMenu->addSeparator();
	_pMenu->addAction(u8"��ͣ�����߳�", this, SLOT(slot_pauseThreads()));
	_pMenu->addAction(u8"���������߳�", this, SLOT(slot_recoverThreads()));
	_pMenu->addAction(u8"��ͣ�߳�", this, SLOT(slot_pauseThread()));
	_pMenu->addAction(u8"�����߳�", this, SLOT(slot_recoverThread()));
	_pMenu->addAction(u8"�˳��߳�", this, SLOT(slot_quitThread()));
}


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



void InfoViewThread::enmuInfo(unsigned int pid)
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
		_vecInfo.clear();
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
				_vecInfo.append(ti);
			}
			CloseHandle(hThread);
		} while (Thread32Next(Snapshot, &te32)); //�ж���һ���߳̿����Ƿ���Ч����Ч�����ѭ��
	}
	//��β����
	CloseHandle(Snapshot); //�رտ��վ��
}

void InfoViewThread::updateView(unsigned int pid)
{
	_pModel->clear();

	// ���ñ�ͷ����
	QStringList headerData;
	headerData
		<< QStringLiteral("�߳�ID")
		<< QStringLiteral("���ȼ�")
		<< QStringLiteral("��ʼ��ַ");
	_pModel->setHorizontalHeaderLabels(headerData);

	

	//����Item����
	int nTo = -1;
	int nCount = 0;
	for (int i = 0; i < _vecInfo.size(); ++i)
	{
		// ���ñ������
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

	this->setWindowTitle(u8"�̣߳�" + QString::number(nCount));
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

		QString strMsg = u8"�Ƿ�Ҫ�˳����߳�";
		int click = QMessageBox::warning(NULL, u8"����", strMsg, QMessageBox::Yes | QMessageBox::No);
		if (click == QMessageBox::Yes) {
			if (::TerminateThread(handle, 0)) {
				_pModel->removeRow(curIndex.row());
			}
		}
		CloseHandle(handle);
	}
}
