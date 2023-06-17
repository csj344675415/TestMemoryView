#pragma once

#include <QPushButton>

class IconButton : public QPushButton
{
	Q_OBJECT

public:
	IconButton(QWidget *parent = Q_NULLPTR);
	~IconButton();

private:
	void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE; //绘制事件
	void resizeEvent(QResizeEvent *event);
	void enterEvent(QEvent *); //鼠标移入事件
	void leaveEvent(QEvent *); //鼠标移出事件
	void mousePressEvent(QMouseEvent *event);   //鼠标按下
	void mouseReleaseEvent(QMouseEvent *event); //鼠标释放

	void drawBg(QPainter *painter);
	void drawIcon(QPainter *painter);
	bool m_isEnter = false;
	bool m_isPress = false;

	QPixmap m_icon;
	int m_offsetX = 0;
	int m_offsetY = 0;
	int m_offsetW = 0;
	int m_offsetH = 0;
	QRect m_rect;
	Qt::TransformationMode m_TransformationMode = Qt::SmoothTransformation;

signals:
	//void clicked(); 
public:
	//设置图标
	void setIcon(QString iconPath) { m_icon = QPixmap(iconPath); }
	void setIcon(QPixmap icon) { m_icon = icon; }
	//设置图标绘制坐标偏移
	void setIconOffsetXY(int x, int y) { m_offsetX = x; m_offsetY = y; }
	//设置图标缩放宽高偏移
	void setIconOffsetWH(int w, int h) { m_offsetW = w; m_offsetH = h; }
	//设置图标抗锯齿
	void setIconRenderHint(bool b) { m_TransformationMode = b ? Qt::SmoothTransformation : Qt::FastTransformation; }
};
