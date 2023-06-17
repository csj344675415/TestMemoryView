#include "HexWidgets.h"
#include <QDragEnterEvent>
#include <QMimeData>

HexWidgets::HexWidgets(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

#if _WIN64
	this->resize(700, 445);
#else
	this->resize(620, 445);
#endif

	
	setAcceptDrops(true);

	_pHexEdit = new HexEdit(this);
	this->setCentralWidget(_pHexEdit);

	connect(_pHexEdit, &HexEdit::signal_updateStatusBar, this, &HexWidgets::slot_updateStatusBar);

	initMenu();
	initStatusBar();
}

HexWidgets::~HexWidgets()
{
}

void HexWidgets::closeEvent(QCloseEvent *event)
{
	event->accept();
	_pHexEdit->updateStop();	//停止数据刷新
}

void HexWidgets::initMenu()
{

}

void HexWidgets::show()
{
	_pHexEdit->updateStart();
	QWidget::show();
}

void HexWidgets::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls())
		event->accept();
}


void HexWidgets::dropEvent(QDropEvent *event)
{
	if (event->mimeData()->hasUrls())
	{
		QList<QUrl> urls = event->mimeData()->urls();
		//QString filePath = urls.at(0).toLocalFile();
		//loadFile(filePath);
		event->accept();
	}
}

void HexWidgets::setData(unsigned int pid, unsigned long long addr)
{ 
	_pHexEdit->setData(pid, addr);
}

#include <QLabel>
void HexWidgets::initStatusBar()
{
	_pLabel_StatusBar_1 = new QLabel(this);
	_pLabel_StatusBar_byte = new QLabel(this);
	_pLabel_StatusBar_2byte = new QLabel(this);
	_pLabel_StatusBar_4byte = new QLabel(this);
	_pLabel_StatusBar_8byte = new QLabel(this);
	_pLabel_StatusBar_float = new QLabel(this);
	_pLabel_StatusBar_double = new QLabel(this);
	ui.statusBar->addWidget(_pLabel_StatusBar_1);
	ui.statusBar->addWidget(_pLabel_StatusBar_byte);
	ui.statusBar->addWidget(_pLabel_StatusBar_2byte);
	ui.statusBar->addWidget(_pLabel_StatusBar_4byte);
	ui.statusBar->addWidget(_pLabel_StatusBar_8byte);
	ui.statusBar->addWidget(_pLabel_StatusBar_float);
	ui.statusBar->addWidget(_pLabel_StatusBar_double);
}

void HexWidgets::slot_updateStatusBar(DataInfo info)
{
	QString strAddr1, strAddr2, strAddr3;
	strAddr1 = QString("%1").arg(info.beginAddr, 8, 16, QChar('0')).toUpper();
	strAddr2 = QString("%1").arg(info.endAddr, 8, 16, QChar('0')).toUpper();

	//PLOGD << "beginAddr = " << info.beginAddr << "\tendAddr = " << info.endAddr;


	if (info.beginAddr == info.endAddr) {
		_pLabel_StatusBar_1->setText(strAddr1);
	}
	else {
		strAddr3 = QString(u8"%1 - %2 (%3字节)").arg(strAddr1).arg(strAddr2).arg(info.byteNum);
		_pLabel_StatusBar_1->setText(strAddr3);
	}

	QString str;

	str = QObject::tr("<font color = #569CD6 >byte: </font>%1").arg(QString::number(*(char*)&info.data[0]));
	_pLabel_StatusBar_byte->setText(str);

	str = QObject::tr("<font color = #569CD6 >short: </font>%1").arg(QString::number(*(short*)&info.data[0]));
	_pLabel_StatusBar_2byte->setText(str);

	str = QObject::tr("<font color = #569CD6 >int: </font>%1").arg(QString::number(*(int*)&info.data[0]));
	_pLabel_StatusBar_4byte->setText(str);

	//str = QObject::tr("<font color = #569CD6 >int64: </font>%1").arg(QString::number(*(long long*)&info.data[0]));
	//_pLabel_StatusBar_8byte->setText(str);


	QFontMetrics fontWidth(this->font());
	str = QObject::tr("<font color = #569CD6 >float: </font>");
	QString str2 = QString::number(*(float*)&info.data[0], 'f', 2);
	int width = fontWidth.width(str2);
	int maxWidth = fontWidth.width(QLatin1String("2147483648"));
	str2 = fontWidth.elidedText(str2, Qt::ElideRight, maxWidth);
	_pLabel_StatusBar_float->setText(str + str2);


	//str2 = QString::number(*(double*)&info.data[0], 'd', 2);
	//str = QObject::tr("<font color = #569CD6 >double: </font>");
	//width = fontWidth.width(str2);
	//maxWidth = fontWidth.width(QLatin1String("-9223372036854775808"));
	//str2 = fontWidth.elidedText(str2, Qt::ElideRight, maxWidth);
	//_pLabel_StatusBar_double->setText(str + str2);


	_pLabel_StatusBar_8byte->hide();
	_pLabel_StatusBar_double->hide();
}

void HexWidgets::updeteView()
{
	_pHexEdit->updeteView();
}