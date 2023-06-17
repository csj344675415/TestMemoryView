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
	//д��ע���
	QSettings settings("QT_VS_Project", "Config");
	settings.beginGroup("PMTWidgets"); //����
	settings.setValue("PrcessStyle", __nStyle);
	settings.endGroup();
}

void PMTWidgets::InitPMT()
{
	setMinimumSize(250, 300); //���ô�����С�ߴ�
	setMaximumSize(250, 300); //���ô������ߴ� 
	setWindowFlags(Qt::WindowCloseButtonHint); //����������ʾ�رհ�ť 
	setWindowIcon(QIcon(":/InjectionTool/MyWidgets/Resources/Process.png"));

	// ��ʼ��View
	InitTableView();
	// ����Ȩ��
	EnablePrivilege();


	//��ȡע���
	QSettings settings("QT_VS_Project", "Config");
	settings.beginGroup("PMTWidgets"); //����
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

	// ����������
	__pView->setSortingEnabled(false);
	// ��ʼ����������������
	__pView->sortByColumn(0, Qt::AscendingOrder);
	// ѡ��ʱ���и���
	__pView->setSelectionBehavior(QAbstractItemView::SelectRows);
	// �Ƿ���ʾ����������
	__pView->setShowGrid(true);
	// �Ƿ������л�ɫ
	__pView->setAlternatingRowColors(false);
	// ˮƽ��ͷ�Ƿ�ɼ����еı�ͷ��
	__pView->horizontalHeader()->setVisible(true);
	// ��ֱ��ͷ�Ƿ�ɼ����еı�ͷ��
	__pView->verticalHeader()->setVisible(false);
	//���ر�ͷ����ָʾ��
	__pView->horizontalHeader()->setSortIndicatorShown(false);
	// ��ֹ�Զ�����
	__pView->setAutoScroll(false);
	// �����ع�������һ�����
	__pView->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
	//����ˮƽ������
	//__pView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	//���ش�ֱ������
	//__pView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);	
	// ���һ���Զ�ռ��������µ����в���
	__pView->horizontalHeader()->setStretchLastSection(true);
	// ���ò����ͼ���С
	__pView->setIconSize(QSize(16, 16));
	// ����Ĭ���и�
	__pView->verticalHeader()->setDefaultSectionSize(24);
	// ���õ�ѡ
	__pView->setSelectionMode(QAbstractItemView::SingleSelection);
	// ���ò��ɱ༭
	//__pView->setEditTriggers(QTableView::NoEditTriggers);
	// �Ƿ�ʹ���Ҽ������˵�
	__pView->setContextMenuPolicy(Qt::CustomContextMenu);
	// �Ҽ������˵����ź�
	connect(__pView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(slot_customContextMenuRequested(const QPoint &)));
	// �����Ԫ�� 
	connect(__pView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(slot_clickedView(const QModelIndex &)));
	// ��Ԫ��˫��ʱ����
	connect(__pView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slot_doubleView(const QModelIndex &)));



	// ʵ������ģ��
	__pProxyModel = new QSortFilterProxyModel(__pView);
	__pModel = new MyAbstractTableModel(__pView, __pProxyModel);
	// ������Ҫ���������Դģ��
	__pProxyModel->setSourceModel(__pModel);
	// ���л�ɫ
	//pModel->setAlternatingRowColors(true);
	
	// ʵ�����Զ����ͷ�����ڱ�ͷ���Ƹ�ѡ��ť
	__pMyHeaderView = new MyHeaderView(Qt::Horizontal, this);
	// ��ͷ�����������	
	__pMyHeaderView->setMinimumSectionSize(30);				//���ñ�ͷ��С���
	__pMyHeaderView->setStretchLastSection(true);				//���һ����ͷ�Զ�ռ��������µ����в���
	__pMyHeaderView->setHighlightSections(true);				//��ͷ�������
	__pMyHeaderView->setDefaultAlignment(Qt::AlignLeft);		//���ñ�ͷ�ı����뷽ʽ
	// �����Զ����ͷ
	__pView->setHorizontalHeader(__pMyHeaderView);
	// �����ź�
	//connect(pModel, SIGNAL(signal_stateChanged(int)), pMyHeaderView, SLOT(slot_stateChanged(int)));	//���ѡ��ť״̬�ı�֪ͨ��ͷ
	//connect(pMyHeaderView, SIGNAL(signal_stateChanged(int)), pModel, SLOT(slot_StateChanged(int)));	//��ͷ��ѡ��ť״̬�ı�֪ͨ���

	__pView->setModel(__pProxyModel);

	createMenu();
}

void PMTWidgets::createMenu()
{
	//if (type == 0) {
	//	__pMenu_Pro = new QMenu(__pView);
	//	__pMenu_Pro->addAction(u8"ѡ�����", this, SLOT(slot_select()));				//ˢ��
	//	__pMenu_Pro->addSeparator();
	//	__pMenu_Pro->addAction(u8"ˢ��", this, SLOT(slot_update()));				//ˢ��
	//	auto aaa = __pMenu_Pro->addAction(u8"���˽���", this, SLOT(slot_filter()));				//ˢ��
	//	aaa->setCheckable(true);
	//	aaa->setChecked(true);
	//	__pMenu_Pro->addSeparator();
	//	__pMenu_Pro->addAction(u8"���Ʊ������", this, SLOT(slot_copy()));			//�򿪽���Ŀ¼
	//	__pMenu_Pro->addAction(u8"�򿪽���Ŀ¼", this, SLOT(slot_openDir()));			//�򿪽���Ŀ¼
	//	__pMenu_Pro->addSeparator();												//�ָ���
	//	__pMenu_Pro->addAction(u8"��ͣ����", this, SLOT(slot_pauseProcess()));		//��ͣ����
	//	__pMenu_Pro->addAction(u8"�ָ�����", this, SLOT(slot_recoverProcess()));	//���н���
	//	__pMenu_Pro->addAction(u8"��������", this, SLOT(slot_quitProcess()));		//��������
	//	__pMenu_Pro->addSeparator();
	//	auto m = __pMenu_Pro->addMenu(u8"�б���ʽ");
	//	m->addAction(u8"��", this, SLOT(slot_simplePro()));
	//	m->addAction(u8"��ͨ", this, SLOT(slot_ordinaryPro()));
	//	m->addAction(u8"����", this, SLOT(slot_detailedPro()));		
	//}
	//else if (type == 1) {
	//	__pMenu_Mod = new QMenu(__pView);
	//	__pMenu_Mod->addAction(u8"ˢ��", this, SLOT(slot_update()));				//ˢ��
	//	auto aaa = __pMenu_Mod->addAction(u8"����ģ��", this, SLOT(slot_filter()));				//ˢ��
	//	aaa->setCheckable(true);
	//	aaa->setChecked(true);
	//	__pMenu_Mod->addSeparator();
	//	__pMenu_Mod->addAction(u8"���Ʊ������", this, SLOT(slot_copy()));			//�򿪽���Ŀ¼
	//	__pMenu_Mod->addAction(u8"��ģ��Ŀ¼", this, SLOT(slot_openDir()));		//��ģ��Ŀ¼
	//	__pMenu_Mod->addSeparator();												//�ָ���
	//	__pMenu_Mod->addAction(u8"ע��DLL", this, SLOT(slot_injectDll()));			//ע��DLL
	//	__pMenu_Mod->addAction(u8"ж��DLL", this, SLOT(slot_unInjectDll()));		//ж��DLL
	//}
	//else if (type == 2) {
	//	__pMenu_Thr = new QMenu(__pView);
	//	__pMenu_Thr->addAction(u8"ˢ��", this, SLOT(slot_update()));				//ˢ��
	//	__pMenu_Thr->addSeparator();
	//	__pMenu_Thr->addAction(u8"���Ʊ������", this, SLOT(slot_copy()));			//�򿪽���Ŀ¼
	//	__pMenu_Thr->addSeparator();												//�ָ���
	//	__pMenu_Thr->addAction(u8"��ͣ�߳�", this, SLOT(slot_pauseThread()));			//ע��DLL
	//	__pMenu_Thr->addAction(u8"�����߳�", this, SLOT(slot_recoverThread()));		//ж��DLL
	//	__pMenu_Thr->addAction(u8"�˳��߳�", this, SLOT(slot_quitThread()));		//ж��DLL
	//}

	//---
	__pMenu_Pro = new QMenu(__pView);
	__pMenu_Pro->addAction(u8"ѡ�����", this, SLOT(slot_select()));				
	__pMenu_Pro->addSeparator();
	__pMenu_Pro->addAction(u8"ˢ��", this, SLOT(slot_update()));				
	auto aaa = __pMenu_Pro->addAction(u8"���˽���", this, SLOT(slot_filter()));				
	aaa->setCheckable(true);
	aaa->setChecked(true);
	__pMenu_Pro->addSeparator();
	__pMenu_Pro->addAction(u8"���Ʊ������", this, SLOT(slot_copy()));			
	__pMenu_Pro->addAction(u8"�򿪽���Ŀ¼", this, SLOT(slot_openDir()));			
	__pMenu_Pro->addSeparator();												
	__pMenu_Pro->addAction(u8"��ͣ����", this, SLOT(slot_pauseProcess()));		
	__pMenu_Pro->addAction(u8"�ָ�����", this, SLOT(slot_recoverProcess()));	
	__pMenu_Pro->addAction(u8"��������", this, SLOT(slot_quitProcess()));		
	__pMenu_Pro->addSeparator();
	auto m = __pMenu_Pro->addMenu(u8"�б���ʽ");
	m->addAction(u8"��", this, SLOT(slot_simplePro()));
	m->addAction(u8"��ͨ", this, SLOT(slot_ordinaryPro()));
	m->addAction(u8"����", this, SLOT(slot_detailedPro()));

	//---
	__pMenu_Mod = new QMenu(__pView);
	__pMenu_Mod->addAction(u8"ˢ��", this, SLOT(slot_update()));				
	auto aaaa = __pMenu_Mod->addAction(u8"����ģ��", this, SLOT(slot_filter()));				
	aaaa->setCheckable(true);
	aaaa->setChecked(true);
	__pMenu_Mod->addSeparator();
	__pMenu_Mod->addAction(u8"���Ʊ������", this, SLOT(slot_copy()));			
	__pMenu_Mod->addAction(u8"��ģ��Ŀ¼", this, SLOT(slot_openDir()));		
	__pMenu_Mod->addSeparator();												
	__pMenu_Mod->addAction(u8"ע��DLL", this, SLOT(slot_injectDll()));			
	__pMenu_Mod->addAction(u8"ж��DLL", this, SLOT(slot_unInjectDll()));		

	//---
	__pMenu_Thr = new QMenu(__pView);
	__pMenu_Thr->addAction(u8"ˢ��", this, SLOT(slot_update()));				
	__pMenu_Thr->addSeparator();
	__pMenu_Thr->addAction(u8"���Ʊ������", this, SLOT(slot_copy()));			
	__pMenu_Thr->addSeparator();												
	__pMenu_Thr->addAction(u8"��ͣ�����߳�", this, SLOT(slot_pauseThreads()));			
	__pMenu_Thr->addAction(u8"���������߳�", this, SLOT(slot_recoverThreads()));		
	__pMenu_Thr->addAction(u8"��ͣ�߳�", this, SLOT(slot_pauseThread()));			
	__pMenu_Thr->addAction(u8"�����߳�", this, SLOT(slot_recoverThread()));		
	__pMenu_Thr->addAction(u8"�˳��߳�", this, SLOT(slot_quitThread()));		

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

		setMinimumSize(600, 305); //���ô�����С�ߴ�
		setMaximumSize(600, 305); //���ô������ߴ� 
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

		setMinimumSize(600, 305); //���ô�����С�ߴ�
		setMaximumSize(600, 305); //���ô������ߴ� 
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
			setWindowTitle(u8"���̣�" + QString::number(index.row() + 1) + "/" + QString::number(__pModel->getRows()));
			QModelIndex modelIndex = __pModel->getModelIndex(index.row(), 0);
			int n = modelIndex.data(Qt::UserRole + 1).toInt();
			__exeName = __vecPro[n].exeName;
			emit clickedView_p(__vecPro[n]);
		}
		else if (__nViewType == 1) {
			setWindowTitle(u8"ģ�飺" + QString::number(index.row() + 1) + "/" + QString::number(__pModel->getRows()));
			QModelIndex modelIndex = __pModel->getModelIndex(index.row(), 0);
			int n = modelIndex.data(Qt::UserRole + 1).toInt();
			__moduleName = __vecMod[n].moduleName;
			emit clickedView_m(__vecMod[n]);
		}
		else if (__nViewType == 2) {
			setWindowTitle(u8"�̣߳�" + QString::number(index.row() + 1) + "/" + QString::number(__pModel->getRows()));
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
	setMinimumSize(w, h); //���ô�����С�ߴ�
	setMaximumSize(w, h); //���ô������ߴ� 
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
	__pView->scrollTo(__pProxyModel->index(row, 0), QAbstractItemView::PositionAtCenter);	//����ָ������
}


