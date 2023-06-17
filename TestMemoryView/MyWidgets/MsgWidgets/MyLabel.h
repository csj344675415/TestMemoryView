#pragma once

#include <QLabel>

class MyLabel : public QLabel
{
	Q_OBJECT

public:
	MyLabel(QWidget *parent = Q_NULLPTR);
	~MyLabel();

signals:
	void signal_DoubleClick();
protected:
	void timerEvent(QTimerEvent *event); //声明时间
	void mouseDoubleClickEvent(QMouseEvent *event); //鼠标双击事件

	int timerID = 100;
	QString strMsg;
	QColor color;
public:
	//0红色 1绿色
	void showMsg(QString msg, int mod = 0, int s = 10);
};
