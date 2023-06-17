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
	//获取顶层窗口句柄
	HWND hwnd = GetTopWindow(0);
	DWORD nPid;
	//遍历系统窗口Z轴序，获取所有窗口矩形大小
	while (hwnd) {
		DWORD tid = GetWindowThreadProcessId(hwnd, &nPid);
		if (nPid == pid) {
			HWND tmp = hwnd;
			wchar_t title[MAX_PATH]{};
			GetWindowTextW(tmp, title, MAX_PATH);
			if (wcscmp(title, L"") != 0) {
				HWND hwndParent = NULL;
				// 循环查找父窗口，以便保证返回的句柄是最顶层的窗口句柄
				while (tmp != NULL) {
					hwndParent = ::GetParent(tmp);
					if (hwndParent == NULL) {
						return tmp;
					}
					tmp = hwndParent;
				}
			}

			
		}
		// 取得下一个窗口句柄
		hwnd = ::GetNextWindow(hwnd, GW_HWNDNEXT);
	}
	return 0;
}

void PMTWidgets::enmuProcess()
{
	HANDLE hProcessSnapshot = INVALID_HANDLE_VALUE; //进程快照句柄，INVALID_HANDLE_VALUE = -1
	HANDLE hModuleSnapshot = INVALID_HANDLE_VALUE;  //模块快照句柄
	hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); //获取进程快照
	if (hProcessSnapshot == INVALID_HANDLE_VALUE) return;

	//结构体相关定义
	PROCESSENTRY32 pe32;                      //进程相关结构体
	memset(&pe32, 0, sizeof(PROCESSENTRY32)); //初始化结构体置空
	pe32.dwSize = sizeof(PROCESSENTRY32);     //设置结构体大小

											  //遍历进程，显示每个进程的信息
	Process32First(hProcessSnapshot, &pe32); //获取第一个进程信息

	__vecPro.clear();
	int nCount = 0;   //用于记录总进程数
	while (Process32Next(hProcessSnapshot, &pe32)) //获取下一个进程信息
	{
		// 获取进程路径
		wchar_t exePath[MAX_PATH] = { 0 };
		HANDLE hProcess = 0;
		hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
		GetModuleFileNameEx(hProcess, NULL, exePath, MAX_PATH);


		// 图标
		QFileInfo fileInfo(QString::fromWCharArray(exePath)); //传入文件路径，获取文件信息
		QFileIconProvider icon_provider;
		QIcon icon = icon_provider.icon(fileInfo); //设置图标
		// 进程名
		QString exeName = QString::fromWCharArray(pe32.szExeFile);
		// 进程路径
		QString strPath = QString::fromWCharArray(exePath);
		// PID
		unsigned int nPid = pe32.th32ProcessID;
		// 此进程开启的线程数
		unsigned int nThreadCount = pe32.cntThreads;
		// 是否为64位进程
		bool is64bit = is64Process(nPid);
		QString bit = is64bit ? "x64" : "x86";
		// 窗口句柄
		HWND hWnd = pid2Hwnd(nPid);
		// 进程TID
		DWORD dwPid = 0;
		unsigned int nTid = GetWindowThreadProcessId(hWnd, &dwPid);
		// 窗口标题
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

	CloseHandle(hProcessSnapshot); //关闭进程快照句柄
}

void PMTWidgets::showProcessList(int style)
{
	__pModel->clear();

	//设置表头内容
	if (style == 0) {	//简约
		__pModel->setHeaderData(0, u8"进程名");
		__pModel->setHeaderData(1, u8"PID");
	}
	else if (style == 1) {	//普通
		__pModel->setHeaderData(0, u8"进程名");
		__pModel->setHeaderData(1, u8"PID");
		__pModel->setHeaderData(2, u8"窗口标题");

	}
	else if (style == 2) {	//详细
		__pModel->setHeaderData(0, u8"进程名");
		__pModel->setHeaderData(1, u8"PID");
		__pModel->setHeaderData(2, u8"TID");
		__pModel->setHeaderData(3, u8"线程数");
		__pModel->setHeaderData(4, u8"窗口标题");
		__pModel->setHeaderData(5, u8"进程路径");
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


		//设置不显示的数据,列随意填，不要超过列数就行
		__pModel->setUserData(nCount, 0, i);

		QString bit = __vecPro[i].is64bit ? "x64" : "x32";
		QString strShow = "[" + bit + "] " + __vecPro[i].exeName;
		QList<QStandardItem*> items;
		if (style == 0) {	//简约
			__pModel->setItemData(nCount, 0, __vecPro[i].icon, strShow);
			__pModel->setItemData(nCount, 1, QString::number(__vecPro[i].pid));
		}
		else if (style == 1) {	//普通
			__pModel->setItemData(nCount, 0, __vecPro[i].icon, strShow);
			__pModel->setItemData(nCount, 1, QString::number(__vecPro[i].pid));
			__pModel->setItemData(nCount, 2, __vecPro[i].title);
		}
		else if (style == 2) {	//详细
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
		__pView->scrollTo(__pProxyModel->index(nTo, 0), QAbstractItemView::PositionAtCenter);	//滚到指定索引
	}
	else {
		__pView->scrollToBottom();
	}

	if (style == 0) {	//简约
		__pView->setColumnWidth(0, 180);
		__pView->setColumnWidth(1, 60);
		__nW = 250; __nH = 300;
	}
	else if (style == 1) {	//普通
		__pView->setColumnWidth(0, 200);
		__pView->setColumnWidth(1, 80);
		__nW = 600; __nH = 310;
	}
	else if (style == 2) {	//详细
		__pView->setColumnWidth(0, 200);
		__pView->setColumnWidth(1, 80);
		__pView->setColumnWidth(2, 80);
		__pView->setColumnWidth(3, 80);
		__pView->setColumnWidth(4, 300);
		//__pView->setColumnWidth(5, 500);
		__nW = 1240; __nH = 595;
	}
	setMinimumSize(__nW, __nH); //设置窗口最小尺寸
	setMaximumSize(__nW, __nH); //设置窗口最大尺寸 
	__pView->resize(__nW, __nH);

	setWindowTitle(u8"进程：" + QString::number(nCount));

	//发射更新信号
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
		QString strMsg = u8"进程名：\n" + fileinfo.fileName() + u8"\n是否要结束此进程";
		int click = QMessageBox::warning(NULL, u8"警告", strMsg, QMessageBox::Yes | QMessageBox::No);
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
	//setMinimumSize(__nW, __nH); //设置窗口最小尺寸
	//setMaximumSize(__nW, __nH); //设置窗口最大尺寸 
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