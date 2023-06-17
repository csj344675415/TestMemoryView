#pragma once

#include <QMainWindow>
#include "ui_HexWidgets.h"

#include "HexEdit.h"

QT_BEGIN_NAMESPACE
//class QAction;
//class QMenu;
//class QUndoStack;
class QLabel;
class QDragEnterEvent;
class QDropEvent;
QT_END_NAMESPACE

class HexWidgets : public QMainWindow
{
	Q_OBJECT

public:
	HexWidgets(QWidget *parent = Q_NULLPTR);
	~HexWidgets();

//private:
	Ui::HexWidgets ui;

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);
	void closeEvent(QCloseEvent *event);

public slots:
	void slot_updateStatusBar(DataInfo info);
public:
	void setData(unsigned int pid, unsigned long long addr = -1);
	void updeteView();
	void show();
protected: //Κά±£»€
	HexEdit* _pHexEdit = nullptr;

	void initMenu();
	void initStatusBar();
	QLabel* _pLabel_StatusBar_1;
	QLabel* _pLabel_StatusBar_byte;
	QLabel* _pLabel_StatusBar_2byte;
	QLabel* _pLabel_StatusBar_4byte;
	QLabel* _pLabel_StatusBar_8byte;
	QLabel* _pLabel_StatusBar_float;
	QLabel* _pLabel_StatusBar_double;
};
