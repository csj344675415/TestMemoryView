#include "PMTWidgets.h"
#include <QFileInfo>
#include <QFileIconProvider>
#include <QTableView>
#include <QMessageBox>
#include <QDesktopServices>
#include <QList>
#include <QStandardItem>
#include <QProcess>

#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib,"Psapi.lib")

#include "MyAbstractTableModel.h"


void PMTWidgets::setProcessListStyle(int style)
{
	__nStyle = style;
}

//---
bool is64OS()
{
	typedef VOID(WINAPI *LPFN_GetNativeSystemInfo)(__out LPSYSTEM_INFO lpSystemInfo);
	LPFN_GetNativeSystemInfo fnGetNativeSystemInfo = (LPFN_GetNativeSystemInfo)GetProcAddress(GetModuleHandleW(L"kernel32"), "GetNativeSystemInfo");
	if (fnGetNativeSystemInfo) {
		SYSTEM_INFO stInfo = { 0 };
		fnGetNativeSystemInfo(&stInfo);
		if (stInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64
			|| stInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
		{
			return true;
		}
	}
	return false;
}

bool is64Process(unsigned int dwProcessID)
{
	if (!is64OS()) {
		return false;
	}
	else {
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessID);
		if (hProcess) {
			typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
			LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandleW(L"kernel32"), "IsWow64Process");
			if (NULL != fnIsWow64Process)
			{
				BOOL bIsWow64 = false;
				fnIsWow64Process(hProcess, &bIsWow64);
				CloseHandle(hProcess);
				if (bIsWow64) {
					return false;
				}
				else {
					return true;
				}
			}
		}
	}
	return false;
}

HWND pid2Hwnd(DWORD pid)
{
	//��ȡ���㴰�ھ��
	HWND hwnd = GetTopWindow(0);
	DWORD nPid;
	//����ϵͳ����Z���򣬻�ȡ���д��ھ��δ�С
	while (hwnd) {
		DWORD tid = GetWindowThreadProcessId(hwnd, &nPid);
		if (nPid == pid) {
			HWND tmp = hwnd;
			wchar_t title[MAX_PATH]{};
			GetWindowTextW(tmp, title, MAX_PATH);
			if (wcscmp(title, L"") != 0) {
				HWND hwndParent = NULL;
				// ѭ�����Ҹ����ڣ��Ա㱣֤���صľ�������Ĵ��ھ��
				while (tmp != NULL) {
					hwndParent = ::GetParent(tmp);
					if (hwndParent == NULL) {
						return tmp;
					}
					tmp = hwndParent;
				}
			}

			
		}
		// ȡ����һ�����ھ��
		hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT);
	}
	return 0;
}

void PMTWidgets::enmuProcess()
{
	HANDLE hProcessSnapshot = INVALID_HANDLE_VALUE; //���̿��վ����INVALID_HANDLE_VALUE = -1
	HANDLE hModuleSnapshot = INVALID_HANDLE_VALUE;  //ģ����վ��
	hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //��ȡ���̿���
	if (hProcessSnapshot == INVALID_HANDLE_VALUE) return;

	//�ṹ����ض���
	PROCESSENTRY32 pe32;                      //������ؽṹ��
	memset(&pe32, 0, sizeof(PROCESSENTRY32)); //��ʼ���ṹ���ÿ�
	pe32.dwSize = sizeof(PROCESSENTRY32);     //���ýṹ���С

											  //�������̣���ʾÿ�����̵���Ϣ
	Process32First(hProcessSnapshot, &pe32); //��ȡ��һ��������Ϣ

	__vecPro.clear();
	int nCount = 0;   //���ڼ�¼�ܽ�����
	while (Process32Next(hProcessSnapshot, &pe32)) //��ȡ��һ��������Ϣ
	{
		// ��ȡ����·��
		wchar_t exePath[MAX_PATH] = { 0 };
		HANDLE hProcess = 0;
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
		GetModuleFileNameEx(hProcess, NULL, exePath, MAX_PATH);


		// ͼ��
		QFileInfo fileInfo(QString::fromWCharArray(exePath)); //�����ļ�·������ȡ�ļ���Ϣ
		QFileIconProvider icon_provider;
		QIcon icon = icon_provider.icon(fileInfo); //����ͼ��
		// ������
		QString exeName = QString::fromWCharArray(pe32.szExeFile);
		// ����·��
		QString strPath = QString::fromWCharArray(exePath);
		// PID
		unsigned int nPid = pe32.th32ProcessID;
		// �˽��̿������߳���
		unsigned int nThreadCount = pe32.cntThreads;
		// �Ƿ�Ϊ64λ����
		bool is64bit = is64Process(nPid);
		QString bit = is64bit ? "x64" : "x86";
		// ���ھ��
		HWND hWnd = pid2Hwnd(nPid);
		// ����TID
		DWORD dwPid = 0;
		unsigned int nTid = GetWindowThreadProcessId(hWnd, &dwPid);
		// ���ڱ���
		wchar_t title[256]{};
		GetWindowTextW(hWnd, title, 256);
		QString strTitle = QString::fromWCharArray(title);

		QPROCESS_INFO pi;
		pi.icon = icon;
		pi.exeName = exeName;
		pi.exePath = QString::fromWCharArray(exePath);
		pi.hwnd = hWnd;
		pi.is64bit = is64bit;
		pi.pid = nPid;
		pi.threadCount = nThreadCount;
		pi.tid = nTid;
		pi.title = strTitle;
		__vecPro.append(pi);

		CloseHandle(hProcess);
	}

	CloseHandle(hProcessSnapshot); //�رս��̿��վ��
}

void PMTWidgets::showProcessList(int style)
{
	__pModel->clear();

	//���ñ�ͷ����
	if (style == 0) {	//��Լ
		__pModel->setHeaderData(0, u8"������");
		__pModel->setHeaderData(1, u8"PID");
	}
	else if (style == 1) {	//��ͨ
		__pModel->setHeaderData(0, u8"������");
		__pModel->setHeaderData(1, u8"PID");
		__pModel->setHeaderData(2, u8"���ڱ���");

	}
	else if (style == 2) {	//��ϸ
		__pModel->setHeaderData(0, u8"������");
		__pModel->setHeaderData(1, u8"PID");
		__pModel->setHeaderData(2, u8"TID");
		__pModel->setHeaderData(3, u8"�߳���");
		__pModel->setHeaderData(4, u8"���ڱ���");
		__pModel->setHeaderData(5, u8"����·��");
	}

	int nTo = -1;
	int nCount = 0;
	for (int i = 0; i < __vecPro.size(); ++i)
	{
		if (__bFilter) {
			int xx = __vecPro[i].exePath.indexOf("C:\\Windows\\System32");
			int xx2 = __vecPro[i].exePath.indexOf("Microsoft Visual Studi");
			int xx4 = __vecPro[i].exePath.indexOf("C:\\Windows");
			bool xx3 = __vecPro[i].exePath.isEmpty();
			if (xx >= 0 || xx2 >= 0 || xx4 >= 0 || xx3 || __vecPro[i].tid == 0) {
				continue;
			}
		}
		

		if (!__exeName.isEmpty()) {
			if (__exeName == __vecPro[i].exeName) {
				nTo = nCount;
			}
		}


		//���ò���ʾ������,���������Ҫ������������
		__pModel->setUserData(nCount, 0, i);

		QString bit = __vecPro[i].is64bit ? "x64" : "x32";
		QString strShow = "[" + bit + "] " + __vecPro[i].exeName;
		QList<QStandardItem*> items;
		if (style == 0) {	//��Լ
			__pModel->setItemData(nCount, 0, __vecPro[i].icon, strShow);
			__pModel->setItemData(nCount, 1, QString::number(__vecPro[i].pid));
		}
		else if (style == 1) {	//��ͨ
			__pModel->setItemData(nCount, 0, __vecPro[i].icon, strShow);
			__pModel->setItemData(nCount, 1, QString::number(__vecPro[i].pid));
			__pModel->setItemData(nCount, 2, __vecPro[i].title);
		}
		else if (style == 2) {	//��ϸ
			__pModel->setItemData(nCount, 0, __vecPro[i].icon, strShow);
			__pModel->setItemData(nCount, 1, QString::number(__vecPro[i].pid));
			__pModel->setItemData(nCount, 2, QString::number(__vecPro[i].tid));
			__pModel->setItemData(nCount, 3, QString::number(__vecPro[i].threadCount));
			__pModel->setItemData(nCount, 4, __vecPro[i].title);
			__pModel->setItemData(nCount, 5, __vecPro[i].exePath);
		}
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

	if (style == 0) {	//��Լ
		__pView->setColumnWidth(0, 180);
		__pView->setColumnWidth(1, 60);
		__nW = 250; __nH = 300;
	}
	else if (style == 1) {	//��ͨ
		__pView->setColumnWidth(0, 200);
		__pView->setColumnWidth(1, 80);
		__nW = 600; __nH = 310;
	}
	else if (style == 2) {	//��ϸ
		__pView->setColumnWidth(0, 200);
		__pView->setColumnWidth(1, 80);
		__pView->setColumnWidth(2, 80);
		__pView->setColumnWidth(3, 80);
		__pView->setColumnWidth(4, 300);
		//__pView->setColumnWidth(5, 500);
		__nW = 1240; __nH = 595;
	}
	setMinimumSize(__nW, __nH); //���ô�����С�ߴ�
	setMaximumSize(__nW, __nH); //���ô������ߴ� 
	__pView->resize(__nW, __nH);

	setWindowTitle(u8"���̣�" + QString::number(nCount));

	//��������ź�
	emit viewUpdate();
}


void PMTWidgets::slot_openDir()
{
	QModelIndex modelIndex = __pModel->getModelIndex(__nCurRow, 0);
	if (modelIndex.isValid()) {
		int n = modelIndex.data(Qt::UserRole + 1).toInt();	
		QString path;
		if (__nViewType == 0) {
			path = __vecPro[n].exePath;
		}
		else if (__nViewType == 1) {
			path = __vecMod[n].modulePath;	
		}

		QProcess process;
		process.startDetached(QString("explorer /select,\"%1\"").arg(path));
	}
}
void PMTWidgets::slot_pauseProcess()
{
	QModelIndex modelIndex = __pModel->getModelIndex(__nCurRow, 0);
	if (modelIndex.isValid()) {
		int n = modelIndex.data(Qt::UserRole + 1).toInt();
		HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, __vecPro[n].tid);
		SuspendThread(handle);
		CloseHandle(handle);
		for (int i = 0; i < __pModel->getColumns(); ++i) {
			modelIndex = __pModel->getModelIndex(__nCurRow, i);
			__pModel->setTextColor(__nCurRow, i, Qt::red);
		}

	}
}
void PMTWidgets::slot_recoverProcess()
{
	QModelIndex modelIndex = __pModel->getModelIndex(__nCurRow, 0);
	if (modelIndex.isValid()) {
		int n = modelIndex.data(Qt::UserRole + 1).toInt();
		HANDLE handle = OpenThread(THREAD_ALL_ACCESS, FALSE, __vecPro[n].tid);
		ResumeThread(handle);
		CloseHandle(handle);
		for (int i = 0; i < __pModel->getColumns(); ++i) {
			__pModel->setTextColor(__nCurRow, i, nullptr);
		}
	}
}
void PMTWidgets::slot_quitProcess()
{
	QModelIndex modelIndex = __pModel->getModelIndex(__nCurRow, 0);
	if (modelIndex.isValid()) {
		int n = modelIndex.data(Qt::UserRole + 1).toInt();
		HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, __vecPro[n].pid);

		QFileInfo fileinfo = QFileInfo(__vecPro[n].exePath);
		QString strMsg = u8"��������\n" + fileinfo.fileName() + u8"\n�Ƿ�Ҫ�����˽���";
		int click = QMessageBox::warning(NULL, u8"����", strMsg, QMessageBox::Yes | QMessageBox::No);
		if (click == QMessageBox::Yes) {
			if (::TerminateProcess(hProcess, 0)) {
				__pModel->removeRow(__nCurRow);
			}
		}
		CloseHandle(hProcess);
	}
}


void PMTWidgets::showProList()
{
	showProcessList(__nStyle);
	//setMinimumSize(__nW, __nH); //���ô�����С�ߴ�
	//setMaximumSize(__nW, __nH); //���ô������ߴ� 
	//__pView->resize(__nW, __nH);
}
void PMTWidgets::slot_simplePro()
{
	__nW = 250; __nH = 300;
	__nStyle = 0;
	showProList();
}
void PMTWidgets::slot_ordinaryPro()
{
	__nW = 600; __nH = 310;
	__nStyle = 1;
	showProList();
}
void PMTWidgets::slot_detailedPro()
{
	__nW = 1240; __nH = 595;
	__nStyle = 2;
	showProList();
}

void PMTWidgets::slot_select()
{
	QModelIndex modelIndex = __pModel->getModelIndex(__nCurRow, __nCurCol);
	if (modelIndex.isValid()) {
		int n = modelIndex.data(Qt::UserRole + 1).toInt();
		__exeName = __vecPro[n].exeName;
		emit selectProcess(__vecPro[n]);
	}
}