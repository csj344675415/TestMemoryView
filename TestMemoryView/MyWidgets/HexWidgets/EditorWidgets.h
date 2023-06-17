#pragma once

#include <QWidget>
#include "ui_EditorWidgets.h"

enum EditorType {
	eEditorData = 0,	//修改数据
	eToAddr,			//转到地址
	eUpdateTime,		//变动数据染色时长
};


typedef struct DataInfo
{
	unsigned int pid;
	unsigned int byteNum;
	unsigned char data[128];
	unsigned long long beginAddr;
	unsigned long long endAddr;
	bool isHex;
	int dataType;
	bool bSymbol;
};

class EditorWidgets : public QWidget
{
	Q_OBJECT

public:
	EditorWidgets(QWidget *parent = Q_NULLPTR);
	~EditorWidgets();

private:
	Ui::EditorWidgets ui;

signals:
	void signal_changeData(DataInfo info, int size);
	void signal_toAddr(unsigned long long addr);
	void signal_setTime(int ms);

public slots:
void slot_comboBox(int index);
void slot_data(DataInfo info);
void slot_checkBox(bool bCheck);
void slot_pushButton();
void slot_pushButton_4();
void slot_pushButton_5();
void slot_radioButton_TextType(int id);
void slot_lineEdit();
void slot_lineEdit_2();

void slot_closeQWidget();

void slot_addr(unsigned long long addr);
void slot_time(int time);

void slot_comboBox_module(QString text);

public:
	void init();
	void initEditorLayout();
	void showType(DataInfo di, int id = eEditorData);

	int _nEditorType = eEditorData;

	DataInfo _di;
	int _dataType = 0;
	bool _bSymbol = false;

	QButtonGroup* pTextTypeGroup = nullptr;

	void addAddr();

	int _ms = 2000;
	void changeTime();
};

