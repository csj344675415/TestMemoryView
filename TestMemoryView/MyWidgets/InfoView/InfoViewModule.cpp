#include "InfoViewModule.h"
#include <QFileDialog>
#include <QMenu>
#include <QSettings>
#include <QMessageBox>

#include <shellapi.h>
#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib,"Psapi.lib")

InfoViewModule::InfoViewModule(QWidget *parent)
	: InfoViewBase(parent)
{
	resize(600, 305);
	initView();
}

InfoViewModule::~InfoViewModule()
{
	writeSettings();
}

void InfoViewModule::readSettings()
{
	//读取注册表
	QSettings settings("QT_VS_Project", "Config");
	settings.beginGroup("InfoView"); //分组
	__LastFilePath = settings.value("LastOpenedPath_DLLFile").toString();
}
void InfoViewModule::writeSettings()
{
	//写入注册表
	QSettings settings("QT_VS_Project", "Config");
	settings.beginGroup("InfoView"); //分组
	settings.setValue("LastOpenedPath_DLLFile", __LastFilePath);
	settings.endGroup();
}

void InfoViewModule::initMenu()
{
	_pMenu = new QMenu(this);
	_pMenu->addAction(u8"刷新", this, SLOT(slot_update()));
	auto t = _pMenu->addAction(u8"过滤模块", this, SLOT(slot_filter()));
	t->setCheckable(true);
	t->setChecked(true);
	_pMenu->addSeparator();
	_pMenu->addAction(u8"复制表格数据", this, SLOT(slot_copy()));
	_pMenu->addAction(u8"打开模块目录", this, SLOT(slot_openDir()));
	_pMenu->addSeparator();
	_pMenu->addAction(u8"注入DLL", this, SLOT(slot_injectDll()));
	_pMenu->addAction(u8"卸载DLL", this, SLOT(slot_unInjectDll()));
}


void InfoViewModule::enmuInfo(unsigned int pid)
{
	if (pid == 0) return;
	HANDLE hModuleSnapshot = INVALID_HANDLE_VALUE;                      //模块快照句柄
	hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);	//获取指定进程的模块快照，参数2.进程ID
	if (hModuleSnapshot == INVALID_HANDLE_VALUE) return;

	//设置结构体
	MODULEENTRY32W me32;                      //模块结构体
	memset(&me32, 0, sizeof(MODULEENTRY32W)); //初始化结构体置空
	me32.dwSize = sizeof(MODULEENTRY32W);     //设置结构体大小

	//开始遍历  
	if (FALSE == Module32First(hModuleSnapshot, &me32)) {//获取第一个模块信息
		CloseHandle(hModuleSnapshot); //关闭快照句柄
		return;
	}

	_vecInfo.clear();
	do {
		QMODULE_INFO mi;
		mi.moduleName = QString::fromWCharArray(me32.szModule);		//模块名
		mi.modulePath = QString::fromWCharArray(me32.szExePath);	//模块路径
		mi.moduleBaseAddr = (unsigned long long)me32.hModule;		//加载基址
		mi.moduleSize = (unsigned long long)me32.modBaseSize;		//大小	
		mi.gcount = me32.GlblcntUsage;								//全局引用计数
		mi.count = me32.ProccntUsage;								//进程引用计数
		_vecInfo.append(mi);
	} while (Module32Next(hModuleSnapshot, &me32)); //获取下一个模块信息

	//收尾工作
	CloseHandle(hModuleSnapshot); //关闭快照句柄
}

void InfoViewModule::updateView(unsigned int pid)
{
	_pModel->clear();

	//设置表头内容
	QStringList headerData;
	headerData
		<< QStringLiteral("起始地址")
		<< QStringLiteral("模块大小")
		<< QStringLiteral("引用计数")
		<< QStringLiteral("模块路径");
	_pModel->setHorizontalHeaderLabels(headerData);


	//设置Item内容
	int nTo = -1;
	int nCount = 0;
	for (int i = 0; i < _vecInfo.size(); ++i)
	{
		if (_bFilter) {
			int xx = _vecInfo[i].modulePath.indexOf("C:\\Windows\\System32");
			int xx2 = _vecInfo[i].modulePath.indexOf("C:\\Windows\\SYSTEM32");
			int xx3 = _vecInfo[i].modulePath.indexOf("C:\\Windows\\system32");
			if (xx >= 0 || xx2 >= 0 || xx3 >= 0) {
				continue;
			}
		}

		if (!__moduleName.isEmpty()) {
			if (__moduleName == _vecInfo[i].moduleName) {
				nTo = nCount;
			}
		}

		// 设置不显示的数据
		this->_hideData.insert(nCount, i);

		// 设置表格数据
		QList<QStandardItem*> items;
		items
			<< new QStandardItem(QString::number(_vecInfo[i].moduleBaseAddr, 16).toUpper())
			<< new QStandardItem(QString::number(_vecInfo[i].moduleSize, 16).toUpper())
			<< new QStandardItem(QString::number(_vecInfo[i].gcount).toUpper())
			<< new QStandardItem(_vecInfo[i].modulePath);

		nCount++;
		_pModel->appendRow(items);
	}

	if (nTo != -1) {
		this->setFocus();
		this->selectRow(nTo);
		this->scrollTo(_pModel->index(nTo, 0), QAbstractItemView::PositionAtCenter);	//滚到指定索引
	}
	else {
		this->scrollToBottom();
	}

	this->setColumnWidth(0, 100);
	this->setColumnWidth(1, 80);
	this->setColumnWidth(2, 60);
	this->setColumnWidth(4, 300);

	this->setWindowTitle(u8"模块：" + QString::number(nCount));
}


void InfoViewModule::slot_injectDll()
{
	QString dllPath = QFileDialog::getOpenFileName(this, tr(u8"打开文件"), __LastFilePath, tr(u8"DLL文件(*.dll)"));//从上次的路径处打开
	if (!dllPath.isEmpty() && _pid != 0)
	{
		__LastFilePath = dllPath;

		dllPath = QDir::toNativeSeparators(dllPath);
		QByteArray ba = dllPath.toLocal8Bit();
		char *path = ba.data();

		QFileInfo fileinfo = QFileInfo(dllPath);
		QString strMsg = u8"文件名：\n" + fileinfo.fileName() + u8"\n是否要注入此DLL文件？";
		int click = QMessageBox::warning(NULL, u8"警告", strMsg, QMessageBox::Yes | QMessageBox::No);
		if (click == QMessageBox::Yes) {
			Inject_CreateRemoteThread(path, _pid);
			__moduleName = fileinfo.fileName();
			enmuInfo(_pid);
			updateView();
		}
	}
}
void InfoViewModule::slot_unInjectDll()
{
	QModelIndex curIndex = this->currentIndex();;
	if (curIndex.isValid()) {
		int n = this->_hideData.value(curIndex.row()).toInt();
		if (!_vecInfo[n].modulePath.isEmpty() && _pid != 0) {
			QString dllPath = QDir::toNativeSeparators(_vecInfo[n].modulePath);
			QFileInfo fileinfo = QFileInfo(dllPath);
			QByteArray ba = dllPath.toLocal8Bit();
			char *path = ba.data();

			QString strMsg = u8"文件名：\n" + fileinfo.fileName() + u8"\n是否要卸载此DLL文件？";
			int click = QMessageBox::warning(NULL, u8"警告", strMsg, QMessageBox::Yes | QMessageBox::No);
			if (click == QMessageBox::Yes) {
				UnInjectDll(path, _pid);
				__moduleName = _vecInfo[n].moduleName;
				enmuInfo(_pid);
				updateView();
			}
		}
	}
}

bool InfoViewModule::Inject_CreateRemoteThread(const char* pszDllFile, DWORD dwProcessId)
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
HMODULE get获取模块地址2(HANDLE hProsess, wchar_t *moduleName)
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
bool InfoViewModule::UnInjectDll(const char* ptszDllFile, DWORD dwProcessId)
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
	while (dll = get获取模块地址2(hProcess, wdllName))
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


void InfoViewModule::slot_openDir()
{
	QModelIndex curIndex = this->currentIndex();;
	if (curIndex.isValid()) {
		int n = this->_hideData.value(curIndex.row()).toInt();
		ShellExecuteW(NULL, L"open", L"explorer", QString("/select, \"%1\"").arg(_vecInfo[n].modulePath).toStdWString().c_str(), NULL, SW_SHOW);
	}
}

void InfoViewModule::slot_clickedView(const QModelIndex & index)
{
	int n = this->_hideData.value(index.row()).toInt();
	__moduleName = _vecInfo[n].moduleName;
}

void InfoViewModule::toRow(QString text, int col)
{
	int rows = _pModel->rowCount();
	auto items = _pModel->findItems(text, Qt::MatchExactly, col);
	if (items.size()) {
		auto item = items.at(0);
		this->setFocus();
		this->selectRow(item->index().row());
		this->scrollTo(item->index(), QAbstractItemView::PositionAtCenter);	//滚到指定索引
	}
}
