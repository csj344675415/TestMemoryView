#include "MyLabel.h"
#include <QTimerEvent>
#include <QMouseEvent>

MyLabel::MyLabel(QWidget *parent)
	: QLabel(parent)
{

}

MyLabel::~MyLabel()
{
}

void MyLabel::showMsg(QString msg, int mod, int s)
{
	if (mod == 0)
		this->setText("<font color = #ff0000 >" + msg + "</font>");	
	else
		this->setText("<font color = #0000ff >" + msg + "</font>");

	//QPalette palette;
	//palette.setColor(QPalette::Text, color);
	//this->setPalette(palette);
	//this->setText(msg);
	timerID = this->startTimer(s * 1000); //TIMER_TIMEOUT为时间间隔，单位毫秒
}

void MyLabel::timerEvent(QTimerEvent *event) //定义事件
{
	if (event->timerId() == timerID)
	{
		this->setText("");
		killTimer(timerID);
	}
}

void MyLabel::mouseDoubleClickEvent(QMouseEvent *event) //鼠标按下事件
{
	if (event->buttons() & Qt::LeftButton)
	{
		emit signal_DoubleClick();
	}
}