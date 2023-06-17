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
	HANDLE hModuleSnapshot = INVALID_HANDLE_VALUE;                      //ģ����վ��
	hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);	//��ȡָ�����̵�ģ����գ�����2.����ID
	if (hModuleSnapshot == INVALID_HANDLE_VALUE) return ;

	//���ýṹ��
	MODULEENTRY32W me32;                      //ģ��ṹ��
	memset(&me32, 0, sizeof(MODULEENTRY32W)); //��ʼ���ṹ���ÿ�
	me32.dwSize = sizeof(MODULEENTRY32W);     //���ýṹ���С

	//��ʼ����  
	if (FALSE == Module32First(hModuleSnapshot, &me32)) {//��ȡ��һ��ģ����Ϣ
		CloseHandle(hModuleSnapshot); //�رտ��վ��
		return ;
	}

	__vecMod.clear();
	do {
		QMODULE_INFO mi;
		mi.moduleName = QString::fromWCharArray(me32.szModule);		//ģ����
		mi.modulePath = QString::fromWCharArray(me32.szExePath);	//ģ��·��
		mi.moduleBaseAddr = (unsigned long long)me32.hModule;		//���ػ�ַ
		mi.moduleSize = (unsigned long long)me32.modBaseSize;		//��С	
		mi.gcount = me32.GlblcntUsage;								//ȫ�����ü���
		mi.count = me32.ProccntUsage;								//�������ü���
		__vecMod.append(mi);
	} while (Module32Next(hModuleSnapshot, &me32)); //��ȡ��һ��ģ����Ϣ

	//��β����
	CloseHandle(hModuleSnapshot); //�رտ��վ��
	return ;
}

void PMTWidgets::showModList()
{
	__pModel->clear();

	//���ñ�ͷ����
	__pModel->setHeaderData(0, u8"��ʼ��ַ");
	__pModel->setHeaderData(1, u8"ģ���С");
	__pModel->setHeaderData(2, u8"���ü���");
	__pModel->setHeaderData(3, u8"ģ��·��");

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

		//���ò���ʾ������,���������Ҫ������������	 
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
		__pView->scrollTo(__pProxyModel->index(nTo, 0), QAbstractItemView::PositionAtCenter);	//����ָ������
	}
	else {
		__pView->scrollToBottom();
	}

	__pView->setColumnWidth(0, 100);
	__pView->setColumnWidth(1, 80);
	__pView->setColumnWidth(2, 60);
	__pView->setColumnWidth(4, 300);

	setWindowTitle(u8"ģ�飺" + QString::number(nCount));

	//��������ź�
	emit viewUpdate();
}

void PMTWidgets::slot_injectDll()
{
	QString dllPath = QFileDialog::getOpenFileName(this, tr(u8"���ļ�"), __LastFilePath, tr(u8"DLL�ļ�(*.dll)"));//���ϴε�·������
	if (!dllPath.isEmpty() && __pid != 0)
	{
		__LastFilePath = dllPath;

		//д��ע���
		QSettings settings("QT_VS_Project", "Config");
		settings.beginGroup("PMTWidgets"); //����
		settings.setValue("LastDLLFilePath", __LastFilePath);
		settings.endGroup();
		
		dllPath = QDir::toNativeSeparators(dllPath);
		QByteArray ba = dllPath.toLocal8Bit();
		char *path = ba.data();

		QFileInfo fileinfo = QFileInfo(dllPath);
		QString strMsg = u8"�ļ�����\n" + fileinfo.fileName() + u8"\n�Ƿ�Ҫע���DLL�ļ���";
		int click = QMessageBox::warning(NULL, u8"����", strMsg, QMessageBox::Yes | QMessageBox::No);
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

			QString strMsg = u8"�ļ�����\n" + fileinfo.fileName() + u8"\n�Ƿ�Ҫж�ش�DLL�ļ���";
			int click = QMessageBox::warning(NULL, u8"����", strMsg, QMessageBox::Yes | QMessageBox::No);
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
HMODULE get��ȡģ���ַ(HANDLE hProsess, wchar_t *moduleName)
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
	while (dll = get��ȡģ���ַ(hProcess, wdllName))
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