#include "IconButton.h"
#include <QPainter>
#include <QMouseEvent>

IconButton::IconButton(QWidget *parent)
	: QPushButton(parent)
{
	this->resize(32, 32);
	m_icon = QPixmap(":/InjectionTool/Resources/computer monitor scree.ico");

	m_rect = this->rect().adjusted(0, 0, -1, -1);
}

IconButton::~IconButton()
{
}

// ��С�ı��¼�
void IconButton::resizeEvent(QResizeEvent *event)
{
	m_rect = this->rect().adjusted(0, 0, -1, -1);
	QWidget::resizeEvent(event);
}

// ���ƿ���
void IconButton::paintEvent(QPaintEvent *event)
{
	//Q_UNUSED(event);

	//����׼������,���÷����
	QPainter painter(this); //ʵ��������
	painter.setRenderHint(QPainter::SmoothPixmapTransform); //���������



	//drawBg(&painter);
	drawIcon(&painter);
}

//����ͼ��
void IconButton::drawBg(QPainter *painter)
{
	// ����ԭʼ����
	painter->save();
	QBrush brush = painter->background();
	QColor col = brush.color();
	auto b = painter->brush();

	auto pen = painter->pen();
	pen.setWidth(1);
	pen.setColor(Qt::red);
	painter->setPen(pen);


	//if (m_isEnter) {
	//	//col.setRed(col.red() - 10);
	//	//col.setGreen(col.green() - 10);
	//	//col.setBlue(col.blue() - 10);
	//	//b.setColor(col);
	//	//painter->setBrush(b);
	//	painter->drawRect(m_rect);
	//}
	//else {
	//	//b.setColor(col);
	//	//painter->setBrush(b);
	//	painter->drawRect(m_rect);
	//}

	// �ָ�ԭʼ����
	painter->restore();
}

//����ͼ��
void IconButton::drawIcon(QPainter *painter)
{
	// ����ԭʼ����
	painter->save();

	// ��ȡ��ͬ��ʾ���ķֱ��ʱ�������ֹ��ͬ�ֱ�����ͼƬʧ��
	qreal pixelRatio = painter->device()->devicePixelRatioF();
	// ����ͼƬ����ǰ�ֱ����µ���ʾ��С��SmoothTransformationƽ������
	QPixmap pixmap;

	if (m_isEnter) {
		pixmap = m_icon.scaled(QSize((width() + m_offsetW - 3) * pixelRatio, (height() + m_offsetH - 3) *pixelRatio), Qt::IgnoreAspectRatio, m_TransformationMode);
		if (m_isPress) {
			painter->drawPixmap(2 + m_offsetX, 2 + m_offsetY, pixmap);
		}
		else {
			painter->drawPixmap(1 + m_offsetX, 1 + m_offsetY, pixmap);
		}
	}
	else {
		pixmap = m_icon.scaled(QSize((width() + m_offsetW - 1) * pixelRatio, (height() + m_offsetH - 1) *pixelRatio), Qt::IgnoreAspectRatio, m_TransformationMode);
		painter->drawPixmap(0 + m_offsetX, 0 + m_offsetY, pixmap);
	}

	// �ָ�ԭʼ����
	painter->restore();
}

// ��갴���¼�
void IconButton::mousePressEvent(QMouseEvent *event)
{
	if (isEnabled())  //�жϰ�ť�Ƿ����
	{
		if (event->buttons() & Qt::LeftButton) {
			event->accept(); //�����¼�
			m_isPress = true;
			update();
		}
		else {
			event->ignore(); //�����¼�
		}
	}
}

// ����ͷ��¼� - �л�����״̬������toggled()�ź�
void IconButton::mouseReleaseEvent(QMouseEvent *event)
{
	if (isEnabled()) //�жϰ�ť�Ƿ����
	{
		if ((event->button() == Qt::LeftButton)) {
			event->accept();       //�����¼�
			m_isPress = false;
			update();
			emit QPushButton::clicked();
		}
		else {
			event->ignore(); //�����¼�
		}
	}
}

void IconButton::enterEvent(QEvent *)
{
	m_isEnter = true;
}
void IconButton::leaveEvent(QEvent *)
{
	m_isEnter = false;
}
