#include "TestMemoryView.h"


#include "WinInfoWidgets\WinInfo.h"
#include "MemoryWidgets\MemoryWidget.h"

MemoryWidget* pMemoryWidget = nullptr;
HWND g_hWnd;

#include "MemoryWidgets\MemoryView.h"
MemoryView* pMemoryView = nullptr;

TestMemoryView::TestMemoryView(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	setWindowFlags(Qt::WindowCloseButtonHint);	//标题栏仅显示关闭按钮 
	connect(ui.pushButton, &QPushButton::clicked, this, &TestMemoryView::slot_pushButton);
	connect(ui.pushButton_2, &QPushButton::clicked, this, &TestMemoryView::slot_pushButton_2);

	qRegisterMetaType<HWND>("HWND");
	connect(ui.label_WIC, &WinInfo::signal_windowHwnd, this, &TestMemoryView::slot_getWindowInfo);
}

TestMemoryView::~TestMemoryView()
{
	if (pMemoryWidget) {
		delete pMemoryWidget;
	}

	if (pMemoryView) {
		delete pMemoryView;
	}
}

void TestMemoryView::slot_getWindowInfo(HWND hwnd)
{
	g_hWnd = hwnd;
	PWINDOWS_INFOMATION pWi;
	pWi = ui.label_WIC->GetWindowInfo(hwnd);

	m_pid = pWi->pid;

	ui.lineEdit->setText(QString::number(m_pid));
}

#include "ConvertUTF.h"

void TestMemoryView::slot_pushButton()
{
	if (!pMemoryWidget) {
		pMemoryWidget = new MemoryWidget(nullptr);
	}

	pMemoryWidget->show(m_pid);
}

#include <QTextCodec>
void TestMemoryView::slot_pushButton_2()
{
	char16_t sss = 0x2C20;
	

	//char s[] = { 0x20,0x2C };
	char s[] = { 0xB0 ,0x70 ,0xD0 ,0x3B ,0xB5 ,0x70 ,0x20 ,0x2C ,0x0E ,0x76 ,0x01 ,0x00 ,0x00 ,0x00 };

	//QTextDecoder

	QByteArray encodedString = s;
	QTextCodec *codec = QTextCodec::codecForName("UTF-16");
	QTextDecoder *td = codec->makeDecoder();
	QString string = td->toUnicode(s, 2);


	//QString string = codec->toUnicode(encodedString);

	ui.lineEdit_2->setText(string);
}