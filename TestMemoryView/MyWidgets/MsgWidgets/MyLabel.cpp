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
	timerID = this->startTimer(s * 1000); //TIMER_TIMEOUTΪʱ��������λ����
}

void MyLabel::timerEvent(QTimerEvent *event) //�����¼�
{
	if (event->timerId() == timerID)
	{
		this->setText("");
		killTimer(timerID);
	}
}

void MyLabel::mouseDoubleClickEvent(QMouseEvent *event) //��갴���¼�
{
	if (event->buttons() & Qt::LeftButton)
	{
		emit signal_DoubleClick();
	}
}