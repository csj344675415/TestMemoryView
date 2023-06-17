#include "EditorWidgets.h"

#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib,"Psapi.lib")

EditorWidgets::EditorWidgets(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowModality(Qt::ApplicationModal);	//设为模态窗口
	setWindowFlags(Qt::WindowCloseButtonHint);	//标题栏仅显示关闭按钮 

	connect(ui.comboBox, SIGNAL(activated(int)), this, SLOT(slot_comboBox(int)));
	connect(ui.checkBox, SIGNAL(clicked(bool)), this, SLOT(slot_checkBox(bool)));

	connect(ui.lineEdit, &QLineEdit::returnPressed, this, &EditorWidgets::slot_pushButton_yes1);

	connect(ui.pushButton_yes1, &QPushButton::clicked, this, &EditorWidgets::slot_pushButton_yes1);		//ok
	connect(ui.pushButton_no1, &QPushButton::clicked, this, [this] { this->close(); });
	connect(ui.pushButton_yes2, &QPushButton::clicked, this, &EditorWidgets::slot_pushButton_yes2);	//ok
	connect(ui.pushButton_no2, &QPushButton::clicked, this, [this] { this->close(); });
	connect(ui.pushButton_yes3, &QPushButton::clicked, this, &EditorWidgets::slot_pushButton_yes3);	//ok		
	connect(ui.pushButton_no3, &QPushButton::clicked, this, [this] { this->close(); });

	connect(ui.comboBox_addr->lineEdit(), &QLineEdit::returnPressed, this, &EditorWidgets::slot_pushButton_yes2);
	connect(ui.comboBox_module, SIGNAL(activated(QString)), this, SLOT(slot_comboBox_module(QString)));


	//Radio按钮分组
	pTextTypeGroup = new QButtonGroup(this);
	pTextTypeGroup->addButton(ui.radioButton_Ansi, 0); //后面的 数字就是信号中发送给槽函数的ID
	pTextTypeGroup->addButton(ui.radioButton_Unicode, 1);
	pTextTypeGroup->addButton(ui.radioButton_UTF8, 2);
	connect(pTextTypeGroup, QOverload<QAbstractButton*>::of(&QButtonGroup::buttonClicked), this, &EditorWidgets::slot_radioButton_TextType);

	ui.radioButton_Ansi->setEnabled(false);
	ui.radioButton_Unicode->setEnabled(false);
	ui.radioButton_UTF8->setEnabled(false);
	//ui.lineEdit_2->setEnabled(false);



}

EditorWidgets::~EditorWidgets()
{
}

void EditorWidgets::show(DataInfo di, int id)
{
	_di = di;
	_nEditorType = id;

	ui.widget->hide();
	ui.widget_2->hide();
	ui.widget_3->hide();

	switch (id)
	{
	case eEditorData:
		deitData(di);
		setWindowTitle(QStringLiteral("更改地址：") + QString("%1").arg(di.beginAddr, 8, 16, QChar('0')).toUpper());
		this->setMinimumSize(ui.widget->rect().width()+10, ui.widget->rect().height() + 10);
		this->setMaximumSize(ui.widget->rect().width() + 10, ui.widget->rect().height() + 10);
		ui.widget->move(5, 5);
		ui.widget->show();
		break;
	case eToAddr:
		addAddr(di);
		setWindowTitle(QStringLiteral("转到地址："));
		this->setMinimumSize(ui.widget_2->rect().width() + 10, ui.widget_2->rect().height() + 10);
		this->setMaximumSize(ui.widget_2->rect().width() + 10, ui.widget_2->rect().height() + 10);
		ui.widget_2->move(5, 5);
		ui.widget_2->show();
		break;
	case eUpdateTime:
		changeTime(di);
		setWindowTitle(QStringLiteral("单位（毫秒）"));
		this->setMinimumSize(ui.widget_3->rect().width() + 10, ui.widget_3->rect().height() + 10);
		this->setMaximumSize(ui.widget_3->rect().width() + 10, ui.widget_3->rect().height() + 10);
		ui.widget_3->move(5, 5);
		ui.widget_3->show();
		break;
	}

	QWidget::show();
}

enum dataType {
	e1ByteHex = 0,
	e1ByteDec,
	e2ByteHex,
	e2ByteDec,
	e4ByteHex,
	e4ByteDec,
	e8ByteHex,
	e8ByteDec,
	e4ByteFloat,
	e8ByteDouble,
};
void EditorWidgets::deitData(DataInfo &di)
{
	ui.radioButton_Ansi->setEnabled(false);
	ui.radioButton_Unicode->setEnabled(false);
	ui.radioButton_UTF8->setEnabled(false);
	//ui.lineEdit_2->setEnabled(false);

	bool bCheck = ui.checkBox->isChecked();
	int base = bCheck ? 16 : 10;

	QString text;
	switch (di.dataType)
	{
	case e1ByteHex:
		text = QString::number(*(unsigned char*)&di.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(0); break;
	case e1ByteDec:
		text = di.bSymbol ? QString::number(*(char*)&di.data[0], base).toUpper() : QString::number(*(unsigned char*)&di.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(0); break;
	case e2ByteHex:
		text = QString::number(*(unsigned short*)&di.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(1); break;
	case e2ByteDec:
		text = di.bSymbol ? QString::number(*(short*)&di.data[0], base).toUpper() : QString::number(*(unsigned short*)&di.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(1); break;
	case e4ByteHex:
		text = QString::number(*(unsigned int*)&di.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(2); break;
	case e4ByteDec:
		text = di.bSymbol ? QString::number(*(int*)&di.data[0], base).toUpper() : QString::number(*(unsigned int*)&di.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(2); break;
	case e8ByteHex:
		text = QString::number(*(unsigned long long*)&di.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(3); break;
	case e8ByteDec:
		text = di.bSymbol ? QString::number(*(long long*)&di.data[0], base).toUpper() : QString::number(*(unsigned long long*)&di.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(3); break;
	case e4ByteFloat:
		text = bCheck ? QString::number(*(unsigned int*)&_di.data[0], base).toUpper() : QString::number(*(float*)&_di.data[0]); ui.comboBox->setCurrentIndex(4); break;
	case e8ByteDouble:
		text = bCheck ? QString::number(*(unsigned long long*)&_di.data[0], base).toUpper() : QString::number(*(double*)&_di.data[0]); ui.comboBox->setCurrentIndex(5);	break;
	default:break;
	}

	ui.lineEdit->setText(text);
	ui.lineEdit->setFocus();
	ui.lineEdit->selectAll();
}

void EditorWidgets::addAddr(DataInfo &di)
{
	QString addr = QString("%1").arg(_di.beginAddr, 8, 16, QChar('0')).toUpper();

	ui.comboBox_addr->lineEdit()->setText(addr);
	ui.comboBox_addr->lineEdit()->setFocus();
	ui.comboBox_addr->lineEdit()->selectAll();


	struct MODULE_INFO{
		unsigned long long moduleBaseAddr;	//模块起始地址
		QString moduleName;					//模块名
	};

	auto enumModule = [&](unsigned int pid)->QVector<MODULE_INFO> {
		if (pid == 0) return QVector<MODULE_INFO>();
		HANDLE hModuleSnapshot = INVALID_HANDLE_VALUE;                      //模块快照句柄
		hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);	//获取指定进程的模块快照，参数2.进程ID
		if (hModuleSnapshot == INVALID_HANDLE_VALUE) return QVector<MODULE_INFO>();

		//设置结构体
		MODULEENTRY32W me32;                      //模块结构体
		memset(&me32, 0, sizeof(MODULEENTRY32W)); //初始化结构体置空
		me32.dwSize = sizeof(MODULEENTRY32W);     //设置结构体大小

		//开始遍历  
		if (FALSE == Module32First(hModuleSnapshot, &me32)) {//获取第一个模块信息
			CloseHandle(hModuleSnapshot); //关闭快照句柄
			return QVector<MODULE_INFO>();
		}

		QVector<MODULE_INFO> modules;
		do {
			MODULE_INFO mi;
			mi.moduleName = QString::fromWCharArray(me32.szModule);		//模块名
			mi.moduleBaseAddr = (unsigned long long)me32.hModule;		//加载基址
			modules.append(mi);
		} while (Module32Next(hModuleSnapshot, &me32)); //获取下一个模块信息

		//收尾工作
		CloseHandle(hModuleSnapshot); //关闭快照句柄
		return modules;
	};

	QVector<MODULE_INFO> vecModules;
	auto temp = enumModule(di.pid);
	vecModules.swap(temp);
	ui.comboBox_module->clear();
	ui.comboBox_module->addItem(" ");
	QString text;
	int index = 0; bool b = true;
	for (int i = 0; i < vecModules.size(); ++i) {
		text = QString("%1 - ").arg((unsigned long long)vecModules.at(i).moduleBaseAddr, 8, 16, QChar('0')).toUpper() + vecModules.at(i).moduleName;
		ui.comboBox_module->addItem(text);
	}
	ui.comboBox_module->setCurrentIndex(0);
}

void EditorWidgets::changeTime(DataInfo &di)
{
	QString str = QString::number(_ms);
	ui.lineEdit_time->setText(str);
	ui.lineEdit_time->setFocus();
	ui.lineEdit_time->selectAll();
}


void EditorWidgets::slot_pushButton_yes1()
{
	QString str = ui.lineEdit->text();
	//unsigned long long value = 0;
	float f = 0.0;
	double d = 0.0;
	int size = 0;
	bool b;

	auto fun = [&](int nSize) {
		bool isHex = ui.checkBox->isChecked();
		if (isHex) {
			auto value = str.toULongLong(&b, 16);
			memcpy(_di.data, &value, nSize);
		}
		else {
			auto value = str.toULongLong(&b, 10);
			if (!b) {
				auto value2 = str.toLongLong(&b, 10);
				memcpy(_di.data, &value2, nSize);
			}
			else {
				memcpy(_di.data, &value, nSize);
			}
		}
	};


	int index = ui.comboBox->currentIndex();
	switch (index)
	{
	case 0:
		size = 1; fun(size); break;
	case 1:
		size = 2; fun(size); break;
	case 2:
		size = 4; fun(size); break;
	case 3:
		size = 8; fun(size); break;
	case 4:
		f = str.toFloat(&b); memcpy(_di.data, &f, 4); size = 4; break;
	case 5:
		d = str.toDouble(&b); memcpy(_di.data, &d, 8); size = 8; break;
	case 6: {
		if (str.isEmpty()) {
			this->close();
		}

		int id = pTextTypeGroup->checkedId();
		if (id == 0) {
			QByteArray ba = str.toLocal8Bit();
			size = strlen(ba.data()) + 1;
			size = size > 128 ? 128 : size;
			memcpy(_di.data, ba.data(), size);
		}
		else if (id == 1) {
			QByteArray ba = str.toLocal8Bit();
			wchar_t wch[32]{};

			unsigned int len = MultiByteToWideChar(CP_ACP, NULL, ba.data(), -1, NULL, 0);
			MultiByteToWideChar(CP_ACP, NULL, ba.data(), -1, wch, len);

			size = wcslen(wch) * 2 + 2;
			size = size > 128 ? 128 : size;
			memcpy(_di.data, wch, size);
		}
		else if (id == 2) {
			QByteArray ba = str.toUtf8();
			size = strlen(ba.data()) + 1;
			size = size > 128 ? 128 : size;
			memcpy(_di.data, ba.data(), size);
		}
		b = true;
		break;
	}
	case 7: {
		str.remove(QRegExp("\\s"));
		int len = str.length();
		if (len % 2) {
			//QMessageBox::critical(NULL, u8"错误", u8"数组长度不对", QMessageBox::Yes | QMessageBox::No);
			b = false;
		}
		else {
			size = len / 2;
			for (int i = 0; i < size; ++i) {
				QString vvv = str.mid(i * 2, 2);
				unsigned char buf = vvv.toUInt(&b, 16);
				memcpy(&_di.data[i], &buf, 1);
			}
		}
		break;
	}
	default:break;
	}

	if (b) {
		_di.dataSize = size;
		emit signal_changeData(_di);
		this->close();
	}
	else {
		//QMessageBox::critical(NULL, u8"错误", u8"输入有误", QMessageBox::Yes | QMessageBox::No);
		return;
	}
}

void EditorWidgets::slot_pushButton_yes2()
{
	unsigned long long addr;
	QString text = ui.comboBox_addr->lineEdit()->text();

	int iCount = ui.comboBox_addr->count();
	bool b2 = false;
	for (int i = 0; i < iCount; ++i) {
		QString str = ui.comboBox_addr->itemText(i);
		if (text == str) {
			b2 = true;
			break;
		}
	}

	if (!b2) {
		ui.comboBox_addr->insertItem(0, text);
	}


	bool b;
	addr = text.toULongLong(&b, 16);
	if (b) {
		emit signal_changeAddr(addr);
		this->close();
	}
	else {
		//QMessageBox::critical(NULL, u8"错误", u8"跳转失败，地址是否有误？", QMessageBox::Yes | QMessageBox::No);
	}
}
void EditorWidgets::slot_pushButton_yes3()
{
	QString time = ui.lineEdit_time->text();
	int ms = time.toUInt();
	_ms = ms;
	emit signal_changeTime(ms);
	this->close();
}

void EditorWidgets::slot_radioButton_TextType()
{

}

void EditorWidgets::slot_comboBox_module(QString text)
{
	int index = text.indexOf(" -");
	text = text.left(index);
	ui.comboBox_addr->lineEdit()->setText(text);
	ui.comboBox_addr->lineEdit()->setFocus();
}

void EditorWidgets::slot_comboBox(int index)
{
	ui.radioButton_Ansi->setEnabled(false);
	ui.radioButton_Unicode->setEnabled(false);
	ui.radioButton_UTF8->setEnabled(false);

	bool bCheck = ui.checkBox->isChecked();
	int base = bCheck ? 16 : 10;

	QString str;
	switch (index)
	{
	case 0:
		str = _di.bSymbol ? QString::number(*(char*)&_di.data[0], base).toUpper() : QString::number(*(unsigned char*)&_di.data[0], base).toUpper(); break;
	case 1:
		str = _di.bSymbol ? QString::number(*(short*)&_di.data[0], base).toUpper() : QString::number(*(unsigned short*)&_di.data[0], base).toUpper(); break;
	case 2:
		str = _di.bSymbol ? QString::number(*(int*)&_di.data[0], base).toUpper() : QString::number(*(unsigned int*)&_di.data[0], base).toUpper(); break;
	case 3:
		str = _di.bSymbol ? QString::number(*(long long*)&_di.data[0], base).toUpper() : QString::number(*(unsigned long long*)&_di.data[0], base).toUpper(); break;
	case 4:
		str = bCheck ? QString::number(*(unsigned int*)&_di.data[0], base).toUpper() : QString::number(*(float*)&_di.data[0]); break;
	case 5:
		str = bCheck ? QString::number(*(unsigned long long*)&_di.data[0], base).toUpper() : QString::number(*(double*)&_di.data[0]); break;
	case 6: {
		//int id = pTextTypeGroup->checkedId();
		////int len = ui.lineEdit_2->text().toUInt();
		//char sss[32]{};
		//char sss2[128]{};
		//wchar_t sss3[128]{};
		//memcpy(sss, _di.data, 16);
		//if (prc->isUTF8编码(sss)) {
		//	str = sss;
		//	PLOGD << "UTF-8";
		//}
		//else {
		//	memcpy(sss3, _di.data, 16);
		//	str = QString::fromWCharArray(sss3);
		//	PLOGD << "其他编码";
		//}
		str.clear();
		ui.radioButton_Ansi->setEnabled(true);
		ui.radioButton_Unicode->setEnabled(true);
		ui.radioButton_UTF8->setEnabled(true);

	}
			break;
	case 7: {
		//int len = ui.lineEdit_2->text().toUInt();
		//for (int i = 0; i < len; ++i) {
		//	str += QString("%1 ").arg(_di.data[i], 2, 16, QChar('0')).toUpper();
		//}
		//ui.lineEdit_2->setEnabled(true);
		break;
	}
	default: break;
	}

	ui.lineEdit->setText(str);
	ui.lineEdit->setFocus();
	ui.lineEdit->selectAll();
}
void EditorWidgets::slot_checkBox(bool bCheck)
{
	QString str = ui.lineEdit->text();

	bool b = false;
	int index = ui.comboBox->currentIndex();
	switch (index)
	{
	case 0: {
		if (bCheck) {
			str = _di.bSymbol ? QString::number((char)str.toLongLong(&b, 10), 16).toUpper() : QString::number((unsigned char)str.toULongLong(&b, 10), 16).toUpper();
		}
		else {
			str = _di.bSymbol ? QString::number((char)str.toULongLong(&b, 16), 10).toUpper() : QString::number((unsigned char)str.toULongLong(&b, 16), 10).toUpper();
		}
		break;
	}
	case 1: {
		if (bCheck) {
			str = _di.bSymbol ? QString::number((short)str.toLongLong(&b, 10), 16).toUpper() : QString::number((unsigned short)str.toULongLong(&b, 10), 16).toUpper();
		}
		else {
			str = _di.bSymbol ? QString::number((short)str.toULongLong(&b, 16), 10).toUpper() : QString::number((unsigned short)str.toULongLong(&b, 16), 10).toUpper();
		}
		break;
	}
	case 2: {
		if (bCheck) {
			str = _di.bSymbol ? QString::number((int)str.toLongLong(&b, 10), 16).toUpper() : QString::number((unsigned int)str.toULongLong(&b, 10), 16).toUpper();
		}
		else {
			str = _di.bSymbol ? QString::number((int)str.toULongLong(&b, 16), 10).toUpper() : QString::number((unsigned int)str.toULongLong(&b, 16), 10).toUpper();
		}
		break;
	}
	case 3: {
		if (bCheck) {
			str = _di.bSymbol ? QString::number((long long)str.toLongLong(&b, 10), 16).toUpper() : QString::number((unsigned long long)str.toULongLong(&b, 10), 16).toUpper();
		}
		else {
			str = _di.bSymbol ? QString::number((long long)str.toULongLong(&b, 16), 10).toUpper() : QString::number((unsigned long long)str.toULongLong(&b, 16), 10).toUpper();
		}
		break;
	}
	case 4: {
		if (bCheck) {
			float f = str.toFloat();
			str = QString::number(*(unsigned int*)&f, 16).toUpper();
		}
		else {
			unsigned long long f = str.toULongLong(&b, 16);
			str = QString::number(*(float*)&f);
		}
		break;
	}
	case 5: {
		if (bCheck) {
			double d = str.toDouble();
			str = QString::number(*(unsigned long long*)&d, 16).toUpper();
		}
		else {
			unsigned long long d = str.toULongLong(&b, 16);
			str = QString::number(*(double*)&d);
		}
		break;
	}
	default:break;
	}

	//if (bCheck) {
	//	if (index >= 0 && index <= 3) {
	//		ui.lineEdit->setValidator(new QRegExpValidator(QRegExp("[ a-fA-F0-9]+$")));
	//	}
	//	else if (index >= 4 && index <= 5) {
	//		ui.lineEdit->setValidator(new QRegExpValidator(QRegExp("[ -|.|0-9")));
	//	}
	//	else {
	//		ui.lineEdit->setValidator(nullptr);
	//	}
	//}
	//else {
	//	ui.lineEdit->setValidator(nullptr);
	//}


	ui.lineEdit->setText(str);
	ui.lineEdit->setFocus();
	ui.lineEdit->selectAll();
}