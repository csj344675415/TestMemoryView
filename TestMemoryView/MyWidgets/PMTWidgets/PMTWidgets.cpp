#include "PMTWidgets.h"
#include <QApplication>
#include <QSettings>
#include <QTableView>
#include <QMenu>
#include <QClipboard>

#include "MyAbstractTableModel.h"
#include "MyHeaderView.h"

PMTWidgets::PMTWidgets(QWidget *parent)
	: QWidget(parent)
{
	InitPMT();
}

PMTWidgets::PMTWidgets(PMTW_TYPE nViewType, QWidget *parent)
	: QWidget(parent)
	, __nViewType(nViewType)
{
	InitPMT();
}

PMTWidgets::~PMTWidgets()
{
	//写入注册表
	QSettings settings("QT_VS_Project", "Config");
	settings.beginGroup("PMTWidgets"); //分组
	settings.setValue("PrcessStyle", __nStyle);
	settings.endGroup();
}

void PMTWidgets::InitPMT()
{
	setMinimumSize(250, 300); //设置窗口最小尺寸
	setMaximumSize(250, 300); //设置窗口最大尺寸 
	setWindowFlags(Qt::WindowCloseButtonHint); //标题栏仅显示关闭按钮 
	setWindowIcon(QIcon(":/InjectionTool/MyWidgets/Resources/Process.png"));

	// 初始化View
	InitTableView();
	// 提升权限
	EnablePrivilege();


	//读取注册表
	QSettings settings("QT_VS_Project", "Config");
	settings.beginGroup("PMTWidgets"); //分组
	QString v = settings.value("PrcessStyle").toString();
	if (!v.isEmpty()) {
		__nStyle = settings.value("PrcessStyle").toInt();
	}
	else {
		__nStyle = 0;
	}
	__LastFilePath = settings.value("LastDLLFilePath").toString();

	settings.endGroup();
}

void PMTWidgets::InitTableView()
{
	__pView = new QTableView(this);
	__pView->resize(250, 300);

	// 设置排序功能
	__pView->setSortingEnabled(false);
	// 初始按照哪列数据排序
	__pView->sortByColumn(0, Qt::AscendingOrder);
	// 选中时整行高亮
	__pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	// 是否显示背景网格线
	__pView->setShowGrid(true);
	// 是否开启隔行换色
	__pView->setAlternatingRowColors(false);
	// 水平表头是否可见（列的表头）
	__pView->horizontalHeader()->setVisible(true);
	// 垂直表头是否可见（行的表头）
	__pView->verticalHeader()->setVisible(false);
	//隐藏表头排序指示器
	__pView->horizontalHeader()->setSortIndicatorShown(false);
	// 禁止自动滚动
	__pView->setAutoScroll(false);
	// 按像素滚动还是一格滚动
	__pView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	//隐藏水平滚动条
	//__pView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//隐藏垂直滚动条
	//__pView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	
	// 最后一列自动占满表格余下的所有部分
	__pView->horizontalHeader()->setStretchLastSection(true);
	// 设置插入的图标大小
	__pView->setIconSize(QSize(16, 16));
	// 设置默认行高
	__pView->verticalHeader()->setDefaultSectionSize(24);
	// 设置单选
	__pView->setSelectionMode(QAbstractItemView::SingleSelection);
	// 设置不可编辑
	//__pView->setEditTriggers(QTableView::NoEditTriggers);
	// 是否使用右键弹出菜单
	__pView->setContextMenuPolicy(Qt::CustomContextMenu);
	// 右键弹出菜单的信号
	connect(__pView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slot_customContextMenuRequested(const QPoint &)));
	// 点击单元格 
	connect(__pView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slot_clickedView(const QModelIndex &)));
	// 单元格被双击时触发
	connect(__pView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slot_doubleView(const QModelIndex &)));



	// 实例排序模型
	__pProxyModel = new QSortFilterProxyModel(__pView);
	__pModel = new MyAbstractTableModel(__pView, __pProxyModel);
	// 设置需要排序的数据源模型
	__pProxyModel->setSourceModel(__pModel);
	// 隔行换色
	//pModel->setAlternatingRowColors(true);
	
	// 实例化自定义表头，用于表头绘制复选按钮
	__pMyHeaderView = new MyHeaderView(Qt::Horizontal, this);
	// 表头相关属性设置	
	__pMyHeaderView->setMinimumSectionSize(30);				//设置表头最小宽度
	__pMyHeaderView->setStretchLastSection(true);				//最后一个表头自动占满表格余下的所有部分
	__pMyHeaderView->setHighlightSections(true);				//表头字体高亮
	__pMyHeaderView->setDefaultAlignment(Qt::AlignLeft);		//设置表头文本对齐方式
	// 设置自定义表头
	__pView->setHorizontalHeader(__pMyHeaderView);
	// 连接信号
	//connect(pModel, SIGNAL(signal_stateChanged(int)), pMyHeaderView, SLOT(slot_stateChanged(int)));	//表格复选按钮状态改变通知表头
	//connect(pMyHeaderView, SIGNAL(signal_stateChanged(int)), pModel, SLOT(slot_StateChanged(int)));	//表头复选按钮状态改变通知表格

	__pView->setModel(__pProxyModel);

	createMenu();
}

void PMTWidgets::createMenu()
{
	//if (type == 0) {
	//	__pMenu_Pro = new QMenu(__pView);
	//	__pMenu_Pro->addAction(u8"选择进程", this, SLOT(slot_select()));				//刷新
	//	__pMenu_Pro->addSeparator();
	//	__pMenu_Pro->addAction(u8"刷新", this, SLOT(slot_update()));				//刷新
	//	auto aaa = __pMenu_Pro->addAction(u8"过滤进程", this, SLOT(slot_filter()));				//刷新
	//	aaa->setCheckable(true);
	//	aaa->setChecked(true);
	//	__pMenu_Pro->addSeparator();
	//	__pMenu_Pro->addAction(u8"复制表格数据", this, SLOT(slot_copy()));			//打开进程目录
	//	__pMenu_Pro->addAction(u8"打开进程目录", this, SLOT(slot_openDir()));			//打开进程目录
	//	__pMenu_Pro->addSeparator();												//分隔线
	//	__pMenu_Pro->addAction(u8"暂停进程", this, SLOT(slot_pauseProcess()));		//暂停进程
	//	__pMenu_Pro->addAction(u8"恢复进程", this, SLOT(slot_recoverProcess()));	//运行进程
	//	__pMenu_Pro->addAction(u8"结束进程", this, SLOT(slot_quitProcess()));		//结束进程
	//	__pMenu_Pro->addSeparator();
	//	auto m = __pMenu_Pro->addMenu(u8"列表样式");
	//	m->addAction(u8"简单", this, SLOT(slot_simplePro()));
	//	m->addAction(u8"普通", this, SLOT(slot_ordinaryPro()));
	//	m->addAction(u8"完整", this, SLOT(slot_detailedPro()));		
	//}
	//else if (type == 1) {
	//	__pMenu_Mod = new QMenu(__pView);
	//	__pMenu_Mod->addAction(u8"刷新", this, SLOT(slot_update()));				//刷新
	//	auto aaa = __pMenu_Mod->addAction(u8"过滤模块", this, SLOT(slot_filter()));				//刷新
	//	aaa->setCheckable(true);
	//	aaa->setChecked(true);
	//	__pMenu_Mod->addSeparator();
	//	__pMenu_Mod->addAction(u8"复制表格数据", this, SLOT(slot_copy()));			//打开进程目录
	//	__pMenu_Mod->addAction(u8"打开模块目录", this, SLOT(slot_openDir()));		//打开模块目录
	//	__pMenu_Mod->addSeparator();												//分隔线
	//	__pMenu_Mod->addAction(u8"注入DLL", this, SLOT(slot_injectDll()));			//注入DLL
	//	__pMenu_Mod->addAction(u8"卸载DLL", this, SLOT(slot_unInjectDll()));		//卸载DLL
	//}
	//else if (type == 2) {
	//	__pMenu_Thr = new QMenu(__pView);
	//	__pMenu_Thr->addAction(u8"刷新", this, SLOT(slot_update()));				//刷新
	//	__pMenu_Thr->addSeparator();
	//	__pMenu_Thr->addAction(u8"复制表格数据", this, SLOT(slot_copy()));			//打开进程目录
	//	__pMenu_Thr->addSeparator();												//分隔线
	//	__pMenu_Thr->addAction(u8"暂停线程", this, SLOT(slot_pauseThread()));			//注入DLL
	//	__pMenu_Thr->addAction(u8"运行线程", this, SLOT(slot_recoverThread()));		//卸载DLL
	//	__pMenu_Thr->addAction(u8"退出线程", this, SLOT(slot_quitThread()));		//卸载DLL
	//}

	//---
	__pMenu_Pro = new QMenu(__pView);
	__pMenu_Pro->addAction(u8"选择进程", this, SLOT(slot_select()));				
	__pMenu_Pro->addSeparator();
	__pMenu_Pro->addAction(u8"刷新", this, SLOT(slot_update()));				
	auto aaa = __pMenu_Pro->addAction(u8"过滤进程", this, SLOT(slot_filter()));				
	aaa->setCheckable(true);
	aaa->setChecked(true);
	__pMenu_Pro->addSeparator();
	__pMenu_Pro->addAction(u8"复制表格数据", this, SLOT(slot_copy()));			
	__pMenu_Pro->addAction(u8"打开进程目录", this, SLOT(slot_openDir()));			
	__pMenu_Pro->addSeparator();												
	__pMenu_Pro->addAction(u8"暂停进程", this, SLOT(slot_pauseProcess()));		
	__pMenu_Pro->addAction(u8"恢复进程", this, SLOT(slot_recoverProcess()));	
	__pMenu_Pro->addAction(u8"结束进程", this, SLOT(slot_quitProcess()));		
	__pMenu_Pro->addSeparator();
	auto m = __pMenu_Pro->addMenu(u8"列表样式");
	m->addAction(u8"简单", this, SLOT(slot_simplePro()));
	m->addAction(u8"普通", this, SLOT(slot_ordinaryPro()));
	m->addAction(u8"完整", this, SLOT(slot_detailedPro()));

	//---
	__pMenu_Mod = new QMenu(__pView);
	__pMenu_Mod->addAction(u8"刷新", this, SLOT(slot_update()));				
	auto aaaa = __pMenu_Mod->addAction(u8"过滤模块", this, SLOT(slot_filter()));				
	aaaa->setCheckable(true);
	aaaa->setChecked(true);
	__pMenu_Mod->addSeparator();
	__pMenu_Mod->addAction(u8"复制表格数据", this, SLOT(slot_copy()));			
	__pMenu_Mod->addAction(u8"打开模块目录", this, SLOT(slot_openDir()));		
	__pMenu_Mod->addSeparator();												
	__pMenu_Mod->addAction(u8"注入DLL", this, SLOT(slot_injectDll()));			
	__pMenu_Mod->addAction(u8"卸载DLL", this, SLOT(slot_unInjectDll()));		

	//---
	__pMenu_Thr = new QMenu(__pView);
	__pMenu_Thr->addAction(u8"刷新", this, SLOT(slot_update()));				
	__pMenu_Thr->addSeparator();
	__pMenu_Thr->addAction(u8"复制表格数据", this, SLOT(slot_copy()));			
	__pMenu_Thr->addSeparator();												
	__pMenu_Thr->addAction(u8"暂停所有线程", this, SLOT(slot_pauseThreads()));			
	__pMenu_Thr->addAction(u8"运行所有线程", this, SLOT(slot_recoverThreads()));		
	__pMenu_Thr->addAction(u8"暂停线程", this, SLOT(slot_pauseThread()));			
	__pMenu_Thr->addAction(u8"运行线程", this, SLOT(slot_recoverThread()));		
	__pMenu_Thr->addAction(u8"退出线程", this, SLOT(slot_quitThread()));		

}

void PMTWidgets::showProcessView()
{
	if (__nViewType == 0) {
		enmuProcess();
		showProcessList(__nStyle);
		show();
	}
}

void PMTWidgets::showModuleView(unsigned int pid)
{
	if (__nViewType == 1) {
		__pid = pid;
		enmuModule(pid);

		setMinimumSize(600, 305); //设置窗口最小尺寸
		setMaximumSize(600, 305); //设置窗口最大尺寸 
		__pView->resize(600, 305);
		showModList();
		show();
	}
}

void PMTWidgets::showThreadView(unsigned int pid)
{
	if (__nViewType == 2) {
		__pid = pid;
		enmuThread(__pid);

		setMinimumSize(600, 305); //设置窗口最小尺寸
		setMaximumSize(600, 305); //设置窗口最大尺寸 
		__pView->resize(600, 305);
		showThrList();
		show();
	}
}

void PMTWidgets::slot_update()
{
	if (__nViewType == 0) {
		enmuProcess();
		showProcessList(__nStyle);
	}
	else if (__nViewType == 1) {
		enmuModule(__pid);
		showModList();
	}
	else if (__nViewType == 2) {
		enmuThread(__pid);
		showThrList();
	}
}

bool PMTWidgets::EnablePrivilege()
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

void PMTWidgets::slot_customContextMenuRequested(const QPoint &pos)
{
	if (__pView->currentIndex().isValid()) {
		__nCurRow = __pView->currentIndex().row();
		__nCurCol = __pView->currentIndex().column();
		if(__nViewType == 0)
			__pMenu_Pro->exec(cursor().pos());
		else if (__nViewType == 1)
			__pMenu_Mod->exec(cursor().pos());
		else if (__nViewType == 2)
			__pMenu_Thr->exec(cursor().pos());
	
	}
}
//m_moduleName
void PMTWidgets::slot_clickedView(const QModelIndex &index)
{
	if (index.isValid()) {
		if (__nViewType == 0) {
			setWindowTitle(u8"进程：" + QString::number(index.row() + 1) + "/" + QString::number(__pModel->getRows()));
			QModelIndex modelIndex = __pModel->getModelIndex(index.row(), 0);
			int n = modelIndex.data(Qt::UserRole + 1).toInt();
			__exeName = __vecPro[n].exeName;
			emit clickedView_p(__vecPro[n]);
		}
		else if (__nViewType == 1) {
			setWindowTitle(u8"模块：" + QString::number(index.row() + 1) + "/" + QString::number(__pModel->getRows()));
			QModelIndex modelIndex = __pModel->getModelIndex(index.row(), 0);
			int n = modelIndex.data(Qt::UserRole + 1).toInt();
			__moduleName = __vecMod[n].moduleName;
			emit clickedView_m(__vecMod[n]);
		}
		else if (__nViewType == 2) {
			setWindowTitle(u8"线程：" + QString::number(index.row() + 1) + "/" + QString::number(__pModel->getRows()));
		}
	}
}

void PMTWidgets::slot_doubleView(const QModelIndex &index)
{
	if (index.isValid()) {
		if (__nViewType == 0) {
			QModelIndex modelIndex = __pModel->getModelIndex(index.row(), 0);
			int n = modelIndex.data(Qt::UserRole + 1).toInt();
			__exeName = __vecPro[n].exeName;
			emit doubleView_p(__vecPro[n]);
		}
		else if (__nViewType == 1) {
			QModelIndex modelIndex = __pModel->getModelIndex(index.row(), 0);
			int n = modelIndex.data(Qt::UserRole + 1).toInt();
			__moduleName = __vecMod[n].moduleName;
			emit doubleView_m(__vecMod[n]);
		}
		else if (__nViewType == 2) {
			//QModelIndex modelIndex = __pModel->getModelIndex(index.row(), 0);
			//int n = modelIndex.data(Qt::UserRole + 1).toInt();
			//m_moduleName = vecMod[n].moduleName;
			//emit doubleView(vecMod[n]);
		}
	}
}


void PMTWidgets::slot_testButton()
{

}

void PMTWidgets::slot_filter()
{
	__bFilter = !__bFilter;
	if (__nViewType == 0) {
		showProList();
	}
	else if (__nViewType == 1) {
		showModList();
	}
}

void PMTWidgets::slot_copy()
{
	QModelIndex modelIndex = __pModel->getModelIndex(__nCurRow, __nCurCol);
	if (modelIndex.isValid()) {
		QString str = __pModel->data(modelIndex,0).toString();
		QClipboard *board = QApplication::clipboard();
		board->setText(str);
	}
}

void PMTWidgets::setResize(unsigned int w, unsigned int h)
{
	setMinimumSize(w, h); //设置窗口最小尺寸
	setMaximumSize(w, h); //设置窗口最大尺寸 
	__pView->resize(w, h);
}

int PMTWidgets::getRowCount()
{
	return __pModel->getRows();
}

QString PMTWidgets::getData(int row, int col)
{
	QModelIndex m = __pModel->getModelIndex(row, col);
	if (m.isValid()) {
		return m.data().toString();
	}
	return QString();
}

void PMTWidgets::toRow(int row)
{
	__pView->setFocus();
	__pView->selectRow(row);
	__pView->scrollTo(__pProxyModel->index(row, 0), QAbstractItemView::PositionAtCenter);	//滚到指定索引
}


