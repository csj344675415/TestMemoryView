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

// 大小改变事件
void IconButton::resizeEvent(QResizeEvent *event)
{
	m_rect = this->rect().adjusted(0, 0, -1, -1);
	QWidget::resizeEvent(event);
}

// 绘制开关
void IconButton::paintEvent(QPaintEvent *event)
{
	//Q_UNUSED(event);

	//绘制准备工作,启用反锯齿
	QPainter painter(this); //实例化画板
	painter.setRenderHint(QPainter::SmoothPixmapTransform); //开启抗锯齿



	//drawBg(&painter);
	drawIcon(&painter);
}

//绘制图标
void IconButton::drawBg(QPainter *painter)
{
	// 保存原始画板
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

	// 恢复原始画板
	painter->restore();
}

//绘制图标
void IconButton::drawIcon(QPainter *painter)
{
	// 保存原始画板
	painter->save();

	// 获取不同显示器的分辨率比例，防止不同分辨率下图片失真
	qreal pixelRatio = painter->device()->devicePixelRatioF();
	// 缩放图片到当前分辨率下的显示大小，SmoothTransformation平滑处理。
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

	// 恢复原始画板
	painter->restore();
}

// 鼠标按下事件
void IconButton::mousePressEvent(QMouseEvent *event)
{
	if (isEnabled())  //判断按钮是否可用
	{
		if (event->buttons() & Qt::LeftButton) {
			event->accept(); //接受事件
			m_isPress = true;
			update();
		}
		else {
			event->ignore(); //忽略事件
		}
	}
}

// 鼠标释放事件 - 切换开关状态、发射toggled()信号
void IconButton::mouseReleaseEvent(QMouseEvent *event)
{
	if (isEnabled()) //判断按钮是否可用
	{
		if ((event->button() == Qt::LeftButton)) {
			event->accept();       //接受事件
			m_isPress = false;
			update();
			emit QPushButton::clicked();
		}
		else {
			event->ignore(); //忽略事件
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
