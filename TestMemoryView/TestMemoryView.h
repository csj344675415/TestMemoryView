#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_TestMemoryView.h"

class TestMemoryView : public QMainWindow
{
    Q_OBJECT

public:
    TestMemoryView(QWidget *parent = Q_NULLPTR);
	~TestMemoryView();

private:
    Ui::TestMemoryViewClass ui;

	public slots:
	void slot_pushButton();
	void slot_pushButton_2();
	void slot_getWindowInfo(HWND hwnd);

public:
	unsigned int m_pid = 0;
};
