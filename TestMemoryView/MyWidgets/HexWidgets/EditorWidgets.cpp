#include "EditorWidgets.h"
#include <QGroupBox> 
#include <QLineEdit> 
#include <QLabel> 
#include <QPushButton> 
#include <QMessageBox>

#include "ReuseCode.hpp"
extern ReuseCode *prc;

EditorWidgets::EditorWidgets(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowModality(Qt::ApplicationModal);	//设为模态窗口
	setWindowFlags(Qt::WindowCloseButtonHint);	//标题栏仅显示关闭按钮 


	connect(ui.comboBox, SIGNAL(activated(int)), this, SLOT(slot_comboBox(int)));
	connect(ui.checkBox, SIGNAL(clicked(bool)), this, SLOT(slot_checkBox(bool)));

	connect(ui.lineEdit, SIGNAL(returnPressed()), this, SLOT(slot_lineEdit()));
	connect(ui.lineEdit_2, SIGNAL(returnPressed()), this, SLOT(slot_lineEdit_2()));

	connect(ui.pushButton, &QPushButton::clicked, this, &EditorWidgets::slot_pushButton);		//ok
	connect(ui.pushButton_2, &QPushButton::clicked, this, [this] { this->close(); });
	connect(ui.pushButton_3, &QPushButton::clicked, this, [this] { this->close(); });
	connect(ui.pushButton_4, &QPushButton::clicked, this, &EditorWidgets::slot_pushButton_4);	//ok
	connect(ui.pushButton_5, &QPushButton::clicked, this, &EditorWidgets::slot_pushButton_5);	//ok		
	connect(ui.pushButton_6, &QPushButton::clicked, this, [this] { this->close(); });

	connect(ui.comboBox_addr->lineEdit(), SIGNAL(returnPressed()), this, SLOT(slot_pushButton_4()));
	connect(ui.comboBox_module, SIGNAL(activated(QString)), this, SLOT(slot_comboBox_module(QString)));

	
	//Radio按钮分组
	pTextTypeGroup = new QButtonGroup(this);
	pTextTypeGroup->addButton(ui.radioButton_Ansi, 0); //后面的 数字就是信号中发送给槽函数的ID
	pTextTypeGroup->addButton(ui.radioButton_Unicode, 1);
	pTextTypeGroup->addButton(ui.radioButton_UTF8, 2);
	connect(pTextTypeGroup, static_cast<void(QButtonGroup::*)(int)>(&QButtonGroup::buttonClicked), this, &EditorWidgets::slot_radioButton_TextType);

	ui.radioButton_Ansi->setEnabled(false);
	ui.radioButton_Unicode->setEnabled(false);
	ui.radioButton_UTF8->setEnabled(false);
	ui.lineEdit_2->setEnabled(false);

	init();
}

EditorWidgets::~EditorWidgets()
{
}

void EditorWidgets::init()
{
	initEditorLayout();
}

void EditorWidgets::initEditorLayout()
{

}

void EditorWidgets::showType(DataInfo di, int id)
{
	_di = di;
	ui.widget->hide();
	ui.widget_2->hide();
	ui.widget_3->hide();

	switch (id)
	{
	case eEditorData:
		slot_data(di);
		setWindowTitle(QStringLiteral("更改地址：") + QString("%1").arg(_di.beginAddr, 8, 16, QChar('0')).toUpper());
		this->setMinimumSize(275, 145);
		this->setMaximumSize(275, 145);
		ui.widget->move(5, 5);
		ui.widget->show();
		break;
	case eToAddr:
		addAddr();
		setWindowTitle(QStringLiteral("转到地址："));
		this->setMinimumSize(311, 101);
		this->setMaximumSize(311, 101);
		ui.widget_2->move(5, 5);
		ui.widget_2->show();
		break;
	case eUpdateTime:
		changeTime();
		setWindowTitle(QStringLiteral("单位（毫秒）"));
		this->setMinimumSize(186, 72);
		this->setMaximumSize(186, 72);
		ui.widget_3->move(5, 5);
		ui.widget_3->show();
		break;
	}

	show();
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
void EditorWidgets::slot_data(DataInfo info)
{
	ui.radioButton_Ansi->setEnabled(false);
	ui.radioButton_Unicode->setEnabled(false);
	ui.radioButton_UTF8->setEnabled(false);
	ui.lineEdit_2->setEnabled(false);

	_di = info;
	bool bCheck = ui.checkBox->isChecked();
	int base = bCheck ? 16 : 10;
	
	QString str;
	switch (info.dataType)
	{
	case e1ByteHex:
		str = QString::number(*(unsigned char*)&info.data[0], base).toUpper() ; ui.comboBox->setCurrentIndex(0); break;
	case e1ByteDec:
		str = info.bSymbol ? QString::number(*(char*)&info.data[0], base).toUpper() : QString::number(*(unsigned char*)&info.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(0); break;
	case e2ByteHex:
		str = QString::number(*(unsigned short*)&info.data[0], base).toUpper() ; ui.comboBox->setCurrentIndex(1); break;
	case e2ByteDec:
		str = info.bSymbol ? QString::number(*(short*)&info.data[0], base).toUpper() : QString::number(*(unsigned short*)&info.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(1); break;
	case e4ByteHex:
		str = QString::number(*(unsigned int*)&info.data[0], base).toUpper() ; ui.comboBox->setCurrentIndex(2); break;
	case e4ByteDec:
		str = info.bSymbol ? QString::number(*(int*)&info.data[0], base).toUpper() : QString::number(*(unsigned int*)&info.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(2); break;
	case e8ByteHex:
		str = QString::number(*(unsigned long long*)&info.data[0], base).toUpper() ; ui.comboBox->setCurrentIndex(3); break;
	case e8ByteDec:
		str = info.bSymbol ? QString::number(*(long long*)&info.data[0], base).toUpper() : QString::number(*(unsigned long long*)&info.data[0], base).toUpper(); ui.comboBox->setCurrentIndex(3); break;
	case e4ByteFloat: 
		str = bCheck ? QString::number(*(unsigned int*)&_di.data[0], base).toUpper() : QString::number(*(float*)&_di.data[0]); ui.comboBox->setCurrentIndex(4); break;
	case e8ByteDouble: 
		str = bCheck ? QString::number(*(unsigned long long*)&_di.data[0], base).toUpper() : QString::number(*(double*)&_di.data[0]); ui.comboBox->setCurrentIndex(5);	break;
	default:break;
	}

	ui.lineEdit->setText(str);
	ui.lineEdit->setFocus();
	ui.lineEdit->selectAll();
}

void EditorWidgets::slot_comboBox(int index)
{
	ui.radioButton_Ansi->setEnabled(false);
	ui.radioButton_Unicode->setEnabled(false);
	ui.radioButton_UTF8->setEnabled(false);
	ui.lineEdit_2->setEnabled(false);

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
		int len = ui.lineEdit_2->text().toUInt();	
		for (int i = 0; i < len; ++i) {
			str += QString("%1 ").arg(_di.data[i], 2, 16, QChar('0')).toUpper();
		}
		ui.lineEdit_2->setEnabled(true);
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
	case 0:{
		if (bCheck) {
			str = _di.bSymbol ? QString::number((char)str.toLongLong(&b, 10), 16).toUpper() : QString::number((unsigned char)str.toLongLong(&b, 10), 16).toUpper();
		}
		else {
			str = _di.bSymbol ? QString::number((char)str.toULongLong(&b, 16), 10).toUpper() : QString::number((unsigned char)str.toULongLong(&b, 16), 10).toUpper();
		}
		break;
	}	
	case 1: {
		if (bCheck) {
			str = _di.bSymbol ? QString::number((short)str.toLongLong(&b, 10), 16).toUpper() : QString::number((unsigned short)str.toLongLong(&b, 10), 16).toUpper();
		}
		else {
			str = _di.bSymbol ? QString::number((short)str.toULongLong(&b, 16), 10).toUpper() : QString::number((unsigned short)str.toULongLong(&b, 16), 10).toUpper();
		}
		break;
	}
	case 2: {
		if (bCheck) {
			str = _di.bSymbol ? QString::number((int)str.toLongLong(&b, 10), 16).toUpper() : QString::number((unsigned int)str.toLongLong(&b, 10), 16).toUpper();
		}
		else {
			str = _di.bSymbol ? QString::number((int)str.toULongLong(&b, 16), 10).toUpper() : QString::number((unsigned int)str.toULongLong(&b, 16), 10).toUpper();
		}
		break;
	}
	case 3: {
		if (bCheck) {
			str = _di.bSymbol ? QString::number((long long)str.toLongLong(&b, 10), 16).toUpper() : QString::number((unsigned long long)str.toLongLong(&b, 10), 16).toUpper();
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


void EditorWidgets::slot_pushButton()
{
	QString str = ui.lineEdit->text();
	//unsigned long long value = 0;
	float f = 0.0;
	double d = 0.0;
	int size = 0;
	bool b;



	auto fun = [&](int nSize){
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
			prc->c2w(ba.data(), wch);	
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
		emit signal_changeData(_di, size);
		this->close();
	}
	else {
		QMessageBox::critical(NULL, u8"错误", u8"输入有误", QMessageBox::Yes | QMessageBox::No);
		return;
	}
}

void EditorWidgets::slot_radioButton_TextType(int id)
{

}

void EditorWidgets::slot_lineEdit()
{
	slot_pushButton();
}

void EditorWidgets::slot_lineEdit_2()
{
	int index = ui.comboBox->currentIndex();
	slot_comboBox(index);
}

void EditorWidgets::slot_addr(unsigned long long addr)
{

}

void EditorWidgets::slot_time(int time)
{

}


// 确认跳转地址
void EditorWidgets::slot_pushButton_4()
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
		emit signal_toAddr(addr);
		this->close();
	}
	else {
		QMessageBox::critical(NULL, u8"错误", u8"跳转失败，地址是否有误？", QMessageBox::Yes | QMessageBox::No);
	}
}


// 确认改变染色时间
void EditorWidgets::slot_pushButton_5()
{
	QString time = ui.lineEdit_time->text();
	int ms = time.toUInt();
	_ms = ms;
	emit signal_setTime(ms);
	this->close();
}

void EditorWidgets::slot_closeQWidget()
{
	this->close();
}

void EditorWidgets::addAddr()
{
	QString addr = QString("%1").arg(_di.beginAddr, 8, 16, QChar('0')).toUpper();
	
	ui.comboBox_addr->lineEdit()->setText(addr);
	ui.comboBox_addr->lineEdit()->setFocus();
	ui.comboBox_addr->lineEdit()->selectAll();

	auto moduleInfo = prc->enum枚举模块(_di.pid);
	ui.comboBox_module->clear();
	ui.comboBox_module->addItem(" ");
	QString str;
	int index = 0; bool b = true;
	for (int i = 0; i < moduleInfo.size(); ++i) {
		str = QString("%1 - ").arg((DWORD_PTR)moduleInfo[i].moduleBaseAddr, 8, 16, QChar('0')).toUpper() + QString::fromStdWString(moduleInfo[i].moduleName);
		ui.comboBox_module->addItem(str);
	}
	ui.comboBox_module->setCurrentIndex(0);

}

void EditorWidgets::changeTime()
{
	QString str = QString::number(_ms);
	ui.lineEdit_time->setText(str);
	ui.lineEdit_time->setFocus();
	ui.lineEdit_time->selectAll();
}

void EditorWidgets::slot_comboBox_module(QString text)
{
	int index = text.indexOf(" -");
	text = text.left(index);
	ui.comboBox_addr->lineEdit()->setText(text);
	ui.comboBox_addr->lineEdit()->setFocus();
}