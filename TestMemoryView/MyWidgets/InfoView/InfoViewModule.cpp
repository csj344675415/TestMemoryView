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
	//��ȡע���
	QSettings settings("QT_VS_Project", "Config");
	settings.beginGroup("InfoView"); //����
	__LastFilePath = settings.value("LastOpenedPath_DLLFile").toString();
}
void InfoViewModule::writeSettings()
{
	//д��ע���
	QSettings settings("QT_VS_Project", "Config");
	settings.beginGroup("InfoView"); //����
	settings.setValue("LastOpenedPath_DLLFile", __LastFilePath);
	settings.endGroup();
}

void InfoViewModule::initMenu()
{
	_pMenu = new QMenu(this);
	_pMenu->addAction(u8"ˢ��", this, SLOT(slot_update()));
	auto t = _pMenu->addAction(u8"����ģ��", this, SLOT(slot_filter()));
	t->setCheckable(true);
	t->setChecked(true);
	_pMenu->addSeparator();
	_pMenu->addAction(u8"���Ʊ������", this, SLOT(slot_copy()));
	_pMenu->addAction(u8"��ģ��Ŀ¼", this, SLOT(slot_openDir()));
	_pMenu->addSeparator();
	_pMenu->addAction(u8"ע��DLL", this, SLOT(slot_injectDll()));
	_pMenu->addAction(u8"ж��DLL", this, SLOT(slot_unInjectDll()));
}


void InfoViewModule::enmuInfo(unsigned int pid)
{
	if (pid == 0) return;
	HANDLE hModuleSnapshot = INVALID_HANDLE_VALUE;                      //ģ����վ��
	hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);	//��ȡָ�����̵�ģ����գ�����2.����ID
	if (hModuleSnapshot == INVALID_HANDLE_VALUE) return;

	//���ýṹ��
	MODULEENTRY32W me32;                      //ģ��ṹ��
	memset(&me32, 0, sizeof(MODULEENTRY32W)); //��ʼ���ṹ���ÿ�
	me32.dwSize = sizeof(MODULEENTRY32W);     //���ýṹ���С

	//��ʼ����  
	if (FALSE == Module32First(hModuleSnapshot, &me32)) {//��ȡ��һ��ģ����Ϣ
		CloseHandle(hModuleSnapshot); //�رտ��վ��
		return;
	}

	_vecInfo.clear();
	do {
		QMODULE_INFO mi;
		mi.moduleName = QString::fromWCharArray(me32.szModule);		//ģ����
		mi.modulePath = QString::fromWCharArray(me32.szExePath);	//ģ��·��
		mi.moduleBaseAddr = (unsigned long long)me32.hModule;		//���ػ�ַ
		mi.moduleSize = (unsigned long long)me32.modBaseSize;		//��С	
		mi.gcount = me32.GlblcntUsage;								//ȫ�����ü���
		mi.count = me32.ProccntUsage;								//�������ü���
		_vecInfo.append(mi);
	} while (Module32Next(hModuleSnapshot, &me32)); //��ȡ��һ��ģ����Ϣ

	//��β����
	CloseHandle(hModuleSnapshot); //�رտ��վ��
}

void InfoViewModule::updateView(unsigned int pid)
{
	_pModel->clear();

	//���ñ�ͷ����
	QStringList headerData;
	headerData
		<< QStringLiteral("��ʼ��ַ")
		<< QStringLiteral("ģ���С")
		<< QStringLiteral("���ü���")
		<< QStringLiteral("ģ��·��");
	_pModel->setHorizontalHeaderLabels(headerData);


	//����Item����
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

		// ���ò���ʾ������
		this->_hideData.insert(nCount, i);

		// ���ñ������
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
		this->scrollTo(_pModel->index(nTo, 0), QAbstractItemView::PositionAtCenter);	//����ָ������
	}
	else {
		this->scrollToBottom();
	}

	this->setColumnWidth(0, 100);
	this->setColumnWidth(1, 80);
	this->setColumnWidth(2, 60);
	this->setColumnWidth(4, 300);

	this->setWindowTitle(u8"ģ�飺" + QString::number(nCount));
}


void InfoViewModule::slot_injectDll()
{
	QString dllPath = QFileDialog::getOpenFileName(this, tr(u8"���ļ�"), __LastFilePath, tr(u8"DLL�ļ�(*.dll)"));//���ϴε�·������
	if (!dllPath.isEmpty() && _pid != 0)
	{
		__LastFilePath = dllPath;

		dllPath = QDir::toNativeSeparators(dllPath);
		QByteArray ba = dllPath.toLocal8Bit();
		char *path = ba.data();

		QFileInfo fileinfo = QFileInfo(dllPath);
		QString strMsg = u8"�ļ�����\n" + fileinfo.fileName() + u8"\n�Ƿ�Ҫע���DLL�ļ���";
		int click = QMessageBox::warning(NULL, u8"����", strMsg, QMessageBox::Yes | QMessageBox::No);
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

			QString strMsg = u8"�ļ�����\n" + fileinfo.fileName() + u8"\n�Ƿ�Ҫж�ش�DLL�ļ���";
			int click = QMessageBox::warning(NULL, u8"����", strMsg, QMessageBox::Yes | QMessageBox::No);
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
	// ����DLL·����������ֽ���
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

	// ���ں�32�л�ȡLoadLibraryA����ʵ��ַ
	PTHREAD_START_ROUTINE pfnThreadRtn = (PTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleW(L"Kernel32"), "LoadLibraryA");
	if (pfnThreadRtn == NULL)
	{
		printf("[-] Error: ���ں�32���Ҳ���LoadLibrary����.\n");
		return false;
	}

	// ��Զ�̽�����Ϊ·��������ռ�
	LPVOID pszLibFileRemote = (PWSTR)VirtualAllocEx(hProcess, NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
	if (pszLibFileRemote == NULL) {
		return false;
	}

	// ��DLL��·�������Ƶ�Զ�̽��̵�ַ�ռ�
	DWORD n = WriteProcessMemory(hProcess, pszLibFileRemote, (PVOID)pszDllFile, dwSize, NULL);
	if (n == 0) {
		return false;
	}

	// ����һ������LoadLibraryA��Զ���߳�
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, pfnThreadRtn, pszLibFileRemote, 0, NULL);
	if (hThread == NULL) {
		return false;
	}

	// �ȴ�Զ���߳���ֹ
	WaitForSingleObject(hThread, INFINITE);

	// �ͷŰ���DLL·������Զ���ڴ沢�رվ��
	if (pszLibFileRemote != NULL)
		VirtualFreeEx(hProcess, pszLibFileRemote, 0, MEM_DECOMMIT);

	if (hThread != NULL)
		CloseHandle(hThread);

	if (hProcess != NULL)
		CloseHandle(hProcess);

	return true;
}

//---
HMODULE get��ȡģ���ַ2(HANDLE hProsess, wchar_t *moduleName)
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
	//�򿪽���
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
	while (dll = get��ȡģ���ַ2(hProcess, wdllName))
	{
		//���� FreeLibrary ������ַ
		FARPROC lpThreadFun = GetProcAddress(GetModuleHandle(L"Kernel32"), "FreeLibrary");
		if (NULL == lpThreadFun) {
			return false;
		}

		// ����Զ���̵߳��� FreeLibrary  
		HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)lpThreadFun, dll /* ģ���� */, 0, NULL);
		if (NULL == hThread) {
			return false;
		}

		// �ȴ�Զ���߳̽���  
		WaitForSingleObject(hThread, INFINITE);
		// ����  
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
		this->scrollTo(item->index(), QAbstractItemView::PositionAtCenter);	//����ָ������
	}
}
