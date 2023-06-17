#include "PMTWidgets.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QTableView>

#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib,"Psapi.lib")

#include "MyAbstractTableModel.h"

void PMTWidgets::enmuModule(DWORD pid)
{
	if (pid == 0) return;
	HANDLE hModuleSnapshot = INVALID_HANDLE_VALUE;                      //模块快照句柄
	hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);	//获取指定进程的模块快照，参数2.进程ID
	if (hModuleSnapshot == INVALID_HANDLE_VALUE) return ;

	//设置结构体
	MODULEENTRY32W me32;                      //模块结构体
	memset(&me32, 0, sizeof(MODULEENTRY32W)); //初始化结构体置空
	me32.dwSize = sizeof(MODULEENTRY32W);     //设置结构体大小

	//开始遍历  
	if (FALSE == Module32First(hModuleSnapshot, &me32)) {//获取第一个模块信息
		CloseHandle(hModuleSnapshot); //关闭快照句柄
		return ;
	}

	__vecMod.clear();
	do {
		QMODULE_INFO mi;
		mi.moduleName = QString::fromWCharArray(me32.szModule);		//模块名
		mi.modulePath = QString::fromWCharArray(me32.szExePath);	//模块路径
		mi.moduleBaseAddr = (unsigned long long)me32.hModule;		//加载基址
		mi.moduleSize = (unsigned long long)me32.modBaseSize;		//大小	
		mi.gcount = me32.GlblcntUsage;								//全局引用计数
		mi.count = me32.ProccntUsage;								//进程引用计数
		__vecMod.append(mi);
	} while (Module32Next(hModuleSnapshot, &me32)); //获取下一个模块信息

	//收尾工作
	CloseHandle(hModuleSnapshot); //关闭快照句柄
	return ;
}

void PMTWidgets::showModList()
{
	__pModel->clear();

	//设置表头内容
	__pModel->setHeaderData(0, u8"起始地址");
	__pModel->setHeaderData(1, u8"模块大小");
	__pModel->setHeaderData(2, u8"引用计数");
	__pModel->setHeaderData(3, u8"模块路径");

	int nTo = -1;
	int nCount = 0;
	for (int i = 0; i < __vecMod.size(); ++i)
	{
		if (__bFilter) {
			int xx = __vecMod[i].modulePath.indexOf("C:\\Windows\\System32");
			int xx2 = __vecMod[i].modulePath.indexOf("C:\\Windows\\SYSTEM32");
			int xx3 = __vecMod[i].modulePath.indexOf("C:\\Windows\\system32");
			if (xx >= 0 || xx2 >= 0 || xx3 >= 0) {
				continue;
			}
		}

		if (!__moduleName.isEmpty()) {
			if (__moduleName == __vecMod[i].moduleName) {
				nTo = nCount;
			}
		}

		//设置不显示的数据,列随意填，不要超过列数就行	 
		__pModel->setUserData(nCount, 0, i);
		__pModel->setItemData(nCount, 0, QString::number(__vecMod[i].moduleBaseAddr, 16).toUpper());
		__pModel->setItemData(nCount, 1, QString::number(__vecMod[i].moduleSize, 16).toUpper());
		__pModel->setItemData(nCount, 2, QString::number(__vecMod[i].gcount).toUpper());
		__pModel->setItemData(nCount, 3, __vecMod[i].modulePath);

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

	__pView->setColumnWidth(0, 100);
	__pView->setColumnWidth(1, 80);
	__pView->setColumnWidth(2, 60);
	__pView->setColumnWidth(4, 300);

	setWindowTitle(u8"模块：" + QString::number(nCount));

	//发射更新信号
	emit viewUpdate();
}

void PMTWidgets::slot_injectDll()
{
	QString dllPath = QFileDialog::getOpenFileName(this, tr(u8"打开文件"), __LastFilePath, tr(u8"DLL文件(*.dll)"));//从上次的路径处打开
	if (!dllPath.isEmpty() && __pid != 0)
	{
		__LastFilePath = dllPath;

		//写入注册表
		QSettings settings("QT_VS_Project", "Config");
		settings.beginGroup("PMTWidgets"); //分组
		settings.setValue("LastDLLFilePath", __LastFilePath);
		settings.endGroup();
		
		dllPath = QDir::toNativeSeparators(dllPath);
		QByteArray ba = dllPath.toLocal8Bit();
		char *path = ba.data();

		QFileInfo fileinfo = QFileInfo(dllPath);
		QString strMsg = u8"文件名：\n" + fileinfo.fileName() + u8"\n是否要注入此DLL文件？";
		int click = QMessageBox::warning(NULL, u8"警告", strMsg, QMessageBox::Yes | QMessageBox::No);
		if (click == QMessageBox::Yes) {
			Inject_CreateRemoteThread(path, __pid);
			__moduleName = fileinfo.fileName();
			enmuModule(__pid);
			showModList();
		}	
	}
}
void PMTWidgets::slot_unInjectDll()
{ 
	QModelIndex modelIndex = __pModel->getModelIndex(__nCurRow, 0);
	if (modelIndex.isValid()) {
		int n = modelIndex.data(Qt::UserRole + 1).toInt();
		if (!__vecMod[n].modulePath.isEmpty() && __pid != 0) {
			QString dllPath = QDir::toNativeSeparators(__vecMod[n].modulePath);
			QFileInfo fileinfo = QFileInfo(dllPath);
			QByteArray ba = dllPath.toLocal8Bit();
			char *path = ba.data();

			QString strMsg = u8"文件名：\n" + fileinfo.fileName() + u8"\n是否要卸载此DLL文件？";
			int click = QMessageBox::warning(NULL, u8"警告", strMsg, QMessageBox::Yes | QMessageBox::No);
			if (click == QMessageBox::Yes) {
				UnInjectDll(path, __pid);
				__moduleName = __vecMod[n].moduleName;
				enmuModule(__pid);
				showModList();
			}
		}
	}
}

bool PMTWidgets::Inject_CreateRemoteThread(const char* pszDllFile, DWORD dwProcessId)
{
	// 计算DLL路径名所需的字节数
	DWORD dwSize = (strlen(pszDllFile) + 1);

	// Get process handle passing in the process ID
	HANDLE hProcess = OpenProcess(
		PROCESS_QUERY_INFORMATION |
		PROCESS_CREATE_THREAD |
		PROCESS_VM_OPERATION |
		PROCESS_VM_WRITE,
		FALSE, dwProcessId);
	if (hProcess == NULL) {
		return false;
	}

	// 在内核32中获取LoadLibraryA的真实地址
	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32"), "LoadLibraryA");
	if (pfnThreadRtn == NULL)
	{
		printf("[-] Error: 在内核32中找不到LoadLibrary函数.\n");
		return false;
	}

	// 在远程进程中为路径名分配空间
	LPVOID pszLibFileRemote = (PWSTR)VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	if (pszLibFileRemote == NULL) {
		return false;
	}

	// 将DLL的路径名复制到远程进程地址空间
	DWORD n = WriteProcessMemory(hProcess, pszLibFileRemote, (PVOID)pszDllFile, dwSize, NULL);
	if (n == 0) {
		return false;
	}

	// 创建一个调用LoadLibraryA的远程线程
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, pszLibFileRemote, 0, NULL);
	if (hThread == NULL) {
		return false;
	}

	// 等待远程线程终止
	WaitForSingleObject(hThread, INFINITE);

	// 释放包含DLL路径名的远程内存并关闭句柄
	if (pszLibFileRemote != NULL)
		VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_DECOMMIT);

	if (hThread != NULL)
		CloseHandle(hThread);

	if (hProcess != NULL)
		CloseHandle(hProcess);

	return true;
}

//---
HMODULE get获取模块地址(HANDLE hProsess, wchar_t *moduleName)
{
	HMODULE temp = 0;
	DWORD cbNeeded = 0;
	wchar_t lpFilename[MAX_PATH] = L"";

	BOOL bRetn = ::EnumProcessModulesEx(hProsess, &temp, sizeof(temp), &cbNeeded, LIST_MODULES_ALL);
	if (!moduleName) return temp;
	int nSize = cbNeeded / sizeof(HMODULE);
	HMODULE *pHmodule = new HMODULE[nSize]();
	bRetn = ::EnumProcessModulesEx(hProsess, pHmodule, cbNeeded, &cbNeeded, LIST_MODULES_ALL);

	HMODULE *p = pHmodule;
	for (int i = 0; i < nSize; ++i) {
		GetModuleFileNameEx(hProsess, *p, lpFilename, MAX_PATH);
		wchar_t *index = wcsrchr(lpFilename, L'\\');
		++index;
		if (wcscmp(index, moduleName) == 0) {
			temp = *p;
			delete[] pHmodule;
			return temp;
		}
		p++;
	}
	return 0;
}
bool PMTWidgets::UnInjectDll(const char* ptszDllFile, DWORD dwProcessId)
{
	//打开进程
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);
	if (!hProcess) {
		return false;
	}

	int len = strlen(ptszDllFile);
	char path[256]{};
	strcpy(path, ptszDllFile);
	//-----------
	char* dllName = strrchr(path, '\\');
	++dllName;
	wchar_t *wdllName;
	int len2 = MultiByteToWideChar(CP_ACP, 0, dllName, strlen(dllName), NULL, 0);
	wdllName = new wchar_t[len2 + 2]();
	MultiByteToWideChar(CP_ACP, 0, dllName, strlen(dllName), wdllName, len2);
	//-----------


	HMODULE dll = NULL;
	while (dll = get获取模块地址(hProcess, wdllName))
	{
		//查找 FreeLibrary 函数地址
		FARPROC lpThreadFun = GetProcAddress(GetModuleHandle(L"Kernel32"), "FreeLibrary");
		if (NULL == lpThreadFun) {
			return false;
		}

		// 创建远程线程调用 FreeLibrary  
		HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpThreadFun, dll /* 模块句柄 */, 0, NULL);
		if (NULL == hThread) {
			return false;
		}

		// 等待远程线程结束  
		WaitForSingleObject(hThread, INFINITE);
		// 清理  
		CloseHandle(hThread);
	}

	delete[] wdllName;
	return true;
}