#pragma once

#include <QMainWindow>
#include "ui_MemoryWidget.h"

#include "MemoryView.h"

QT_BEGIN_NAMESPACE
class QCloseEvent;
class QLabel;
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE


class MemoryWidget : public QMainWindow
{
	Q_OBJECT

public:
	MemoryWidget(QWidget *parent = Q_NULLPTR);
	~MemoryWidget();

private:
	Ui::MemoryWidget ui;

	void closeEvent(QCloseEvent *event);

	public slots:
	void slot_updateStatusBar(DataInfo info);
	void slot_resize(int w);
public:
	void show(unsigned int pid, unsigned long long addr = -1);
	void setData(unsigned int pid, unsigned long long addr = -1); 

	MemoryView* pMemoryView = nullptr;

	void initStatusBar();
	QLabel* _pLabel_StatusBar_1;
	QLabel* _pLabel_StatusBar_byte;
	QLabel* _pLabel_StatusBar_2byte;
	QLabel* _pLabel_StatusBar_4byte;
	QLabel* _pLabel_StatusBar_8byte;
	QLabel* _pLabel_StatusBar_float;
	QLabel* _pLabel_StatusBar_double;
};
