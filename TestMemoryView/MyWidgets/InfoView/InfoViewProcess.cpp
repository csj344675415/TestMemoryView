#include "InfoViewProcess.h"
#include <QFileInfo>
#include <QFileIconProvider>
#include <QStandardItemModel>
#include <QMenu>
#include <QSettings>
#include <QMessageBox>

#include <shellapi.h>
#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib,"Psapi.lib")

InfoViewProcess::InfoViewProcess(QWidget *parent)
	: InfoViewBase(parent)
	, _nStyle(0)
{
	initView();
}

InfoViewProcess::~InfoViewProcess()
{
	writeSettings();
}

void InfoViewProcess::readSettings()
{
	//读取注册表
	QSettings settings("QT_VS_Project", "Config");
	settings.beginGroup("InfoView"); //分组
	QString v = settings.value("PrcessStyle").toString();
	if (!v.isEmpty()) {
		_nStyle = v.toInt();
	}
}
void InfoViewProcess::writeSettings()
{
	//写入注册表
	QSettings settings("QT_VS_Project", "Config");
	settings.beginGroup("InfoView"); //分组
	settings.setValue("PrcessStyle", _nStyle);
	settings.endGroup();
}

void InfoViewProcess::initMenu()
{
	_pMenu = new QMenu(this);
	_pMenu->addAction(u8"选择进程", this, SLOT(slot_select()));
	_pMenu->addSeparator();
	_pMenu->addAction(u8"刷新", this, SLOT(slot_update()));
	auto t = _pMenu->addAction(u8"过滤进程", this, SLOT(slot_filter()));
	t->setCheckable(true);
	t->setChecked(true);
	_pMenu->addSeparator();
	_pMenu->addAction(u8"复制表格数据", this, SLOT(slot_copy()));
	_pMenu->addAction(u8"打开进程目录", this, SLOT(slot_openDir()));
	_pMenu->addSeparator();
	_pMenu->addAction(u8"暂停进程", this, SLOT(slot_pauseProcess()));
	_pMenu->addAction(u8"恢复进程", this, SLOT(slot_recoverProcess()));
	_pMenu->addAction(u8"结束进程", this, SLOT(slot_quitProcess()));
	_pMenu->addSeparator();
	auto m = _pMenu->addMenu(u8"列表样式");
	m->addAction(u8"简单", this, SLOT(slot_simplePro()));
	m->addAction(u8"普通", this, SLOT(slot_ordinaryPro()));
	m->addAction(u8"完整", this, SLOT(slot_detailedPro()));

	//readSettings();
}


HWND InfoViewProcess::pid2Hwnd(DWORD pid)
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

void InfoViewProcess::enmuInfo(unsigned int pid)
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

	_vecInfo.clear();
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
		_vecInfo.append(pi);

		CloseHandle(hProcess);
	}

	CloseHandle(hProcessSnapshot); //关闭进程快照句柄
}

void InfoViewProcess::updateView(unsigned int pid)
{
	_pModel->clear();

	//设置表头内容
	QStringList headerData;
	if (_nStyle == 0) {	//简约
		headerData
			<< QStringLiteral("进程名")
			<< QStringLiteral("PID");
	}
	else if (_nStyle == 1) {	//普通
		headerData
			<< QStringLiteral("进程名")
			<< QStringLiteral("PID")
			<< QStringLiteral("窗口标题");
	}
	else if (_nStyle == 2) {	//详细
		headerData
			<< QStringLiteral("进程名")
			<< QStringLiteral("PID")
			<< QStringLiteral("TID")
			<< QStringLiteral("线程数")
			<< QStringLiteral("窗口标题")
			<< QStringLiteral("进程路径");
	}
	_pModel->setHorizontalHeaderLabels(headerData);

	//设置Item内容
	int nTo = -1;
	int nCount = 0;
	for (int i = 0; i < _vecInfo.size(); ++i)
	{
		if (_bFilter) {
			int xx = _vecInfo[i].exePath.indexOf("C:\\Windows\\System32");
			int xx2 = _vecInfo[i].exePath.indexOf("Microsoft Visual Studi");
			int xx4 = _vecInfo[i].exePath.indexOf("C:\\Windows");
			bool xx3 = _vecInfo[i].exePath.isEmpty();
			if (xx >= 0 || xx2 >= 0 || xx4 >= 0 || xx3 || _vecInfo[i].tid == 0) {
				continue;
			}
		}

		if (!__exeName.isEmpty() && __exeName == _vecInfo[i].exeName) {
			nTo = nCount;
		}

		//设置不显示的数据
		this->_hideData.insert(nCount, i);

		QString bit = _vecInfo[i].is64bit ? "x64" : "x32";
		QString exeName = "[" + bit + "] " + _vecInfo[i].exeName;

		QList<QStandardItem*> items;
		if (_nStyle == 0) {	//简约
			items
				<< new QStandardItem(_vecInfo[i].icon, exeName)
				<< new QStandardItem(QString::number(_vecInfo[i].pid));
		}
		else if (_nStyle == 1) {	//普通
			items
				<< new QStandardItem(_vecInfo[i].icon, exeName)
				<< new QStandardItem(QString::number(_vecInfo[i].pid))
				<< new QStandardItem(_vecInfo[i].title);
		}
		else if (_nStyle == 2) {	//详细
			items
				<< new QStandardItem(_vecInfo[i].icon, exeName)
				<< new QStandardItem(QString::number(_vecInfo[i].pid))
				<< new QStandardItem(QString::number(_vecInfo[i].tid))
				<< new QStandardItem(QString::number(_vecInfo[i].threadCount))
				<< new QStandardItem(_vecInfo[i].title)
				<< new QStandardItem(_vecInfo[i].exePath);
		}
		nCount++;
		_pModel->appendRow(items);
	}
	
	// 设置列宽
	if (_nStyle == 0) {	//简约
		this->hideColumn(2);
		this->setColumnWidth(0, 180);
		this->setColumnWidth(1, 60);
		this->resize(250, 300);
	}
	else if (_nStyle == 1) {	//普通
		this->setColumnWidth(0, 200);
		this->setColumnWidth(1, 80);
		this->resize(600, 310);
	}
	else if (_nStyle == 2) {	//详细
		this->setColumnWidth(0, 200);
		this->setColumnWidth(1, 80);
		this->setColumnWidth(2, 80);
		this->setColumnWidth(3, 80);
		this->setColumnWidth(4, 300);
		//__pView->setColumnWidth(5, 500);
		this->resize(1240, 595);
	}

	if (nTo != -1) {
		this->setFocus();
		this->selectRow(nTo);
		this->scrollTo(_pModel->index(nTo, 0), QAbstractItemView::PositionAtCenter);	//滚到指定索引
	}
	else {
		this->scrollToBottom();
	}

	this->setWindowTitle(u8"进程：" + QString::number(nCount));
}



void InfoViewProcess::slot_select()
{
	QModelIndex curIndex = this->currentIndex();;
	if (curIndex.isValid()) {
		//int n = this->_hideData.value(curIndex.row()).toInt();
		//emit signal_selectItemValue(_vecInfo[n].exeName);

		emit signal_selectProcess(curIndex);
	}
}

void InfoViewProcess::slot_openDir()
{
	QModelIndex curIndex = this->currentIndex();;
	if (curIndex.isValid()) {
		int n = this->_hideData.value(curIndex.row()).toInt();
		ShellExecuteW(NULL, L"open", L"explorer", QString("/select, \"%1\"").arg(_vecInfo[n].exePath).toStdWString().c_str(), NULL, SW_SHOW);
	}
	
}
void InfoViewProcess::slot_pauseProcess()
{
	QModelIndex curIndex = this->currentIndex();;
	if (curIndex.isValid()) {
		int n = this->_hideData.value(curIndex.row()).toInt();
		HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, _vecInfo[n].tid);
		SuspendThread(handle);
		CloseHandle(handle);
		for (int i = 0; i < _pModel->columnCount(); ++i) {
			auto item = _pModel->item(curIndex.row(), i);
			if (item) {
				item->setForeground(QColor(Qt::red));
				//item->setData(QColor(Qt::red), Qt::TextColorRole);
			}
		}
	}
}
void InfoViewProcess::slot_recoverProcess()
{
	QModelIndex curIndex = this->currentIndex();;
	if (curIndex.isValid()) {
		int n = this->_hideData.value(curIndex.row()).toInt();
		HANDLE handle = ::OpenThread(THREAD_ALL_ACCESS, FALSE, _vecInfo[n].tid);
		SuspendThread(handle);
		CloseHandle(handle);
		for (int i = 0; i < _pModel->columnCount(); ++i) {
			auto item = _pModel->item(curIndex.row(), i);
			if (item) {
				item->setForeground(QColor(nullptr));
				//item->setData(QColor(nullptr), Qt::TextColorRole);
			}
		}
	}
}
void InfoViewProcess::slot_quitProcess()
{
	QModelIndex curIndex = this->currentIndex();;
	if (curIndex.isValid()) {
		int n = this->_hideData.value(curIndex.row()).toInt();
		HANDLE hProcess = ::OpenProcess(PROCESS_TERMINATE, FALSE, _vecInfo[n].pid);

		QString strMsg = u8"进程名：\n" + _vecInfo[n].exeName + u8"\n是否要结束此进程";
		int click = QMessageBox::warning(NULL, u8"警告", strMsg, QMessageBox::Yes | QMessageBox::No);
		if (click == QMessageBox::Yes) {
			if (::TerminateProcess(hProcess, 0)) {
				_pModel->removeRow(curIndex.row());
			}
		}
		CloseHandle(hProcess);
	}
}

void InfoViewProcess::slot_simplePro()
{
	//__nW = 250; __nH = 300;
	_nStyle = 0;
	updateView();
}
void InfoViewProcess::slot_ordinaryPro()
{
	//__nW = 600; __nH = 310;
	_nStyle = 1;
	updateView();
}
void InfoViewProcess::slot_detailedPro()
{
	//__nW = 1240; __nH = 595;
	_nStyle = 2;
	updateView();
}

void InfoViewProcess::slot_clickedView(const QModelIndex & index)
{
	int n = this->_hideData.value(index.row()).toInt();
	__exeName = _vecInfo[n].exeName;
}

//void InfoViewProcess::slot_doubleView(const QModelIndex & index)
//{
//
//}

QPROCESS_INFO InfoViewProcess::getRowData(const QModelIndex & index)
{
	if (index.isValid()) {
		int n = this->_hideData.value(index.row()).toInt();
		return _vecInfo[n];
	}
	return QPROCESS_INFO();
}