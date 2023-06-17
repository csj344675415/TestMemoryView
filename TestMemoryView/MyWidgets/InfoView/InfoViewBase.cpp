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

	if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &Luid)) //SE_DEBUG_NAME��DBGȨ�ޣ��� SE_SHUTDOWN_NAME���ر�ϵͳȨ�ޣ�
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
	// ����������
	this->setSortingEnabled(false);
	// ��ʼ����������������
	this->sortByColumn(0, Qt::AscendingOrder);
	// ѡ��ʱ���и���
	this->setSelectionBehavior(QAbstractItemView::SelectRows);
	// �Ƿ���ʾ����������
	this->setShowGrid(true);
	// �Ƿ������л�ɫ
	this->setAlternatingRowColors(false);
	// ˮƽ��ͷ�Ƿ�ɼ����еı�ͷ��
	this->horizontalHeader()->setVisible(true);
	// ��ֱ��ͷ�Ƿ�ɼ����еı�ͷ��
	this->verticalHeader()->setVisible(false);
	//���ر�ͷ����ָʾ��
	this->horizontalHeader()->setSortIndicatorShown(false);
	// ��ֹ�Զ�����
	this->setAutoScroll(false);
	// �����ع�������һ�����
	this->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	//����ˮƽ������
	//this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//���ش�ֱ������
	//this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 
	// ���һ���Զ�ռ��������µ����в���
	this->horizontalHeader()->setStretchLastSection(true);
	// ���ò����ͼ���С
	this->setIconSize(QSize(16, 16));
	// ����Ĭ���и�
	this->verticalHeader()->setDefaultSectionSize(24);
	// ���õ�ѡ
	this->setSelectionMode(QAbstractItemView::SingleSelection);
	// ���ò��ɱ༭
	//this->setEditTriggers(QTableView::NoEditTriggers);
	// �Ƿ�ʹ���Ҽ������˵�
	this->setContextMenuPolicy(Qt::CustomContextMenu);
	// �Ҽ������˵����ź�
	connect(this, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slot_customContextMenuRequested(const QPoint &)));
	// �����Ԫ�� 
	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slot_clickedView(const QModelIndex &)));
	// ��Ԫ��˫��ʱ����
	connect(this, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slot_doubleView(const QModelIndex &)));

	_pModel = new InfoViewItemModel(this);
	this->setModel(_pModel);

	initMenu();
	readSettings();
	// ��ʼö����Ҫ����Ϣ����
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