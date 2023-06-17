#include "InfoViewWindows.h"
#include <QStandardItemModel>


InfoViewWindows::InfoViewWindows(QWidget *parent)
	: QTreeView(parent)
{
	resize(1250, 305);
	initTree();
}

InfoViewWindows::~InfoViewWindows()
{
}

void InfoViewWindows::initTree()
{
	_pModel = new QStandardItemModel(this);//创建模型指定父类
	this->setModel(_pModel);
}



typedef struct CALLBACK_ARG
{
	DWORD pid;
	DWORD tid;
	InfoViewWindows* pThis;
	QVector<QWINDOW_INFO> vecInfo;
};


BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam)
{
	CALLBACK_ARG *pca = (CALLBACK_ARG*)lParam;

	DWORD pid;
	DWORD tid = GetWindowThreadProcessId(hwndChild, &pid);


	wchar_t title[MAX_PATH]{};
	wchar_t className[MAX_PATH]{};
	RECT rect;
	HWND hwndParent = NULL;

	hwndParent = ::GetParent(hwndChild);
	GetWindowTextW(hwndChild, title, MAX_PATH);
	GetClassName(hwndChild, className, MAX_PATH);
	GetWindowRect(hwndChild, &rect);						//获得窗口矩形
	//GetClientRect(hwndChild, &rect);						//获得客户区矩形


	QWINDOW_INFO wi{};
	wi.hwnd = (unsigned long long)hwndChild;
	wi.hwndParent = (unsigned long long)hwndParent;
	wi.threadId = tid;
	wi.title = QString::fromWCharArray(title);
	wi.className = QString::fromWCharArray(className);
	wi.rect = QRect(rect.left, rect.top, rect.right, rect.bottom);
	pca->vecInfo.append(wi);

	::EnumChildWindows(hwndChild, EnumChildProc, (LPARAM)pca);

	return TRUE;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	CALLBACK_ARG *pca = (CALLBACK_ARG*)lParam;

	DWORD pid;
	DWORD tid = GetWindowThreadProcessId(hwnd, &pid);

	if(pid == pca->pid){
		wchar_t title[MAX_PATH]{};
		wchar_t className[MAX_PATH]{};
		RECT rect;
		HWND hwndParent = NULL;

		hwndParent = ::GetParent(hwnd);
		GetWindowTextW(hwnd, title, MAX_PATH);
		GetClassName(hwnd, className, MAX_PATH);
		GetWindowRect(hwnd, &rect);						//获得窗口矩形
		//GetClientRect(hwnd, &rect);						//获得客户区矩形


		QWINDOW_INFO wi{};
		wi.hwnd = (unsigned long long)hwnd;
		wi.hwndParent = (unsigned long long)hwndParent;
		wi.threadId = tid;
		wi.title = QString::fromWCharArray(title);
		wi.className = QString::fromWCharArray(className);
		wi.rect = QRect(rect.left, rect.top, rect.right, rect.bottom);
		pca->vecInfo.append(wi);

		::EnumChildWindows(hwnd, EnumChildProc, (LPARAM)pca);
	}
	return TRUE;
}

void InfoViewWindows::enmuInfo(unsigned int pid)
{
	if (pid == 0) return;

	CALLBACK_ARG ca;
	ca.pid = pid;
	ca.pThis = this;


	::EnumWindows(EnumWindowsProc, (LPARAM)&ca);

	_vecInfo.swap(ca.vecInfo);
}


void InfoViewWindows::updateView(unsigned int pid)
{
	_pModel->clear();

	// 设置表头内容
	QStringList headerData;
	headerData
		
		<< QStringLiteral("窗口句柄")
		<< QStringLiteral("父窗口句柄")
		<< QStringLiteral("线程ID")
		<< QStringLiteral("窗口标题")
		<< QStringLiteral("窗口类名")
		<< QStringLiteral("窗口大小");
	_pModel->setHorizontalHeaderLabels(headerData);


	for (auto &t : _vecInfo) {
		if (t.hwndParent == 0) {
			QList<QStandardItem*> items;
			items

				<< new QStandardItem(QString::number(t.hwnd, 16).toUpper())
				<< new QStandardItem(QString::number(t.hwndParent, 16).toUpper())
				<< new QStandardItem(QString::number(t.threadId))
				<< new QStandardItem(t.title)
				<< new QStandardItem(t.className)
				<< new QStandardItem(QString("%1, %2, %3, %4").arg(t.rect.x()).arg(t.rect.y()).arg(t.rect.width()).arg(t.rect.height()));
			_pModel->appendRow(items);
		}
	}
		

	for (auto &t : _vecInfo) {
		if(t.hwndParent == 0) continue;
		QList<QStandardItem*> items;
		items
			
			<< new QStandardItem(QString::number(t.hwnd, 16).toUpper())
			<< new QStandardItem(QString::number(t.hwndParent, 16).toUpper())
			<< new QStandardItem(QString::number(t.threadId))
			<< new QStandardItem(t.title)
			<< new QStandardItem(t.className)
			<< new QStandardItem(QString("%1, %2, %3, %4").arg(t.rect.x()).arg(t.rect.y()).arg(t.rect.width()).arg(t.rect.height()));


		QList<QStandardItem*> it = _pModel->findItems(QString::number(t.hwndParent, 16).toUpper(), Qt::MatchRecursive, 0);
		if (it.size() == 0) {	
			_pModel->appendRow(items);
		}
		else{
			it.at(0)->appendRow(items);


			//QModelIndex index = _pModel->indexFromItem(it.at(0));
			//QStandardItem *ttt = _pModel->item(index.row(), 0);
			//if (ttt) {
			//	ttt->appendRow(items);
			//}
			
			
		}
	}


	this->setColumnWidth(0, 150);
	this->setColumnWidth(1, 80);
	this->setColumnWidth(2, 60);
	this->setColumnWidth(3, 300);
	this->setColumnWidth(4, 300);

}

void InfoViewWindows::showInfoTree(unsigned int pid)
{
	enmuInfo(pid);
	updateView(pid);
	show();
}