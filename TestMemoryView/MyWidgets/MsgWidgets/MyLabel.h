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
	void timerEvent(QTimerEvent *event); //����ʱ��
	void mouseDoubleClickEvent(QMouseEvent *event); //���˫���¼�

	int timerID = 100;
	QString strMsg;
	QColor color;
public:
	//0��ɫ 1��ɫ
	void showMsg(QString msg, int mod = 0, int s = 10);
};
