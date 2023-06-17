#pragma once

#include <QWidget>
#include "ui_EditorWidgets.h"
#include <QButtonGroup>

#ifndef STRUCT_EditorWidgets
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
	unsigned int dataSize;
	unsigned long long beginAddr;
	unsigned long long endAddr;
	bool isHex;
	int dataType;
	bool bSymbol;

	DataInfo() {
		this->pid = 0;
		this->byteNum = 0;
		this->dataSize = 0;
		this->beginAddr = 0;
		this->endAddr = 0;
		this->isHex = false;
		this->dataType = 0;
		this->bSymbol = false;
	}

	bool isValid() {
		return pid==0?false:true;
	}
};
#define STRUCT_EditorWidgets
#endif

class EditorWidgets : public QWidget
{
	Q_OBJECT

public:
	EditorWidgets(QWidget *parent = Q_NULLPTR);
	~EditorWidgets();

private:
	Ui::EditorWidgets ui;

signals:
	void signal_changeData(DataInfo di);
	void signal_changeTime(int ms);
	void signal_changeAddr(unsigned long long addr);
	
public slots:
void slot_pushButton_yes1();
void slot_pushButton_yes2();
void slot_pushButton_yes3();
void slot_radioButton_TextType();
void slot_comboBox_module(QString text);
void slot_comboBox(int index);
void slot_checkBox(bool bCheck);


public:
	void show(DataInfo di, int id = eEditorData);
	int _nEditorType = eEditorData;
	DataInfo _di;

	void deitData(DataInfo &di);
	void addAddr(DataInfo &di);
	void changeTime(DataInfo &di);
	int _ms = 1000;

	QButtonGroup* pTextTypeGroup = nullptr;
};
