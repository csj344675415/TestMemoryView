#include "InfoViewBase.h"
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QClipboard>

InfoViewBase::InfoViewBase(QWidget *parent)
	: QTableView(parent)
	, _pModel(nullptr)
	, _bFilter(true)
{
	EnablePrivilege();
}

InfoViewBase::~InfoViewBase()
{
}

bool InfoViewBase::EnablePrivilege()
{
	HANDLE hToken;
	LUID Luid;
	TOKEN_PRIVILEGES tp;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))return false;

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid)) //SE_DEBUG_NAME（DBG权限）， SE_SHUTDOWN_NAME（关闭系统权限）
	{
		CloseHandle(hToken);
		return false;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = Luid;
	tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if (!AdjustTokenPrivileges(hToken, false, &tp, sizeof(tp), NULL, NULL))
	{
		CloseHandle(hToken);
		return false;
	}
	CloseHandle(hToken);

	return true;
}

void InfoViewBase::initView()
{
	// 设置排序功能
	this->setSortingEnabled(false);
	// 初始按照哪列数据排序
	this->sortByColumn(0, Qt::AscendingOrder);
	// 选中时整行高亮
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	// 是否显示背景网格线
	this->setShowGrid(true);
	// 是否开启隔行换色
	this->setAlternatingRowColors(false);
	// 水平表头是否可见（列的表头）
	this->horizontalHeader()->setVisible(true);
	// 垂直表头是否可见（行的表头）
	this->verticalHeader()->setVisible(false);
	//隐藏表头排序指示器
	this->horizontalHeader()->setSortIndicatorShown(false);
	// 禁止自动滚动
	this->setAutoScroll(false);
	// 按像素滚动还是一格滚动
	this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	//隐藏水平滚动条
	//this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//隐藏垂直滚动条
	//this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 
	// 最后一列自动占满表格余下的所有部分
	this->horizontalHeader()->setStretchLastSection(true);
	// 设置插入的图标大小
	this->setIconSize(QSize(16, 16));
	// 设置默认行高
	this->verticalHeader()->setDefaultSectionSize(24);
	// 设置单选
	this->setSelectionMode(QAbstractItemView::SingleSelection);
	// 设置不可编辑
	//this->setEditTriggers(QTableView::NoEditTriggers);
	// 是否使用右键弹出菜单
	this->setContextMenuPolicy(Qt::CustomContextMenu);
	// 右键弹出菜单的信号
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slot_customContextMenuRequested(const QPoint &)));
	// 点击单元格 
	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slot_clickedView(const QModelIndex &)));
	// 单元格被双击时触发
	connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slot_doubleView(const QModelIndex &)));

	_pModel = new InfoViewItemModel(this);
	this->setModel(_pModel);

	initMenu();
	readSettings();
	// 开始枚举需要的信息数据
	//enmuInfo();
	//updateView();
}

void InfoViewBase::slot_customContextMenuRequested(const QPoint & pos)
{
	if (this->currentIndex().isValid() && _pMenu) {
		_pMenu->exec(cursor().pos());
	}
}
void InfoViewBase::slot_clickedView(const QModelIndex & index)
{
	emit signal_clickedView(index);
}
void InfoViewBase::slot_doubleView(const QModelIndex & index)
{
	emit signal_doubleView(index);
}

//QModelIndex InfoViewBase::getIndex()
//{
//	return this->currentIndex();
//}

void InfoViewBase::slot_update()
{
	enmuInfo(_pid);
	updateView();
	emit  signal_updateView();
}

void InfoViewBase::slot_filter()
{
	_bFilter = !_bFilter;
	updateView();
}

void InfoViewBase::slot_copy()
{
	QModelIndex curIndex = this->currentIndex();
	if (curIndex.isValid()) {
		QString str = _pModel->data(curIndex, 0).toString();
		QClipboard *board = QApplication::clipboard();
		board->setText(str);
	}
}

void InfoViewBase::showInfoView(unsigned int pid)
{
	_pid = pid;
	slot_update();
	show();
}

//---
bool InfoViewBase::is64OS()
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

bool InfoViewBase::is64Process(unsigned int dwProcessID)
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