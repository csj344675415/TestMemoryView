#pragma once

#include <QPushButton>

class IconButton : public QPushButton
{
	Q_OBJECT

public:
	IconButton(QWidget *parent = Q_NULLPTR);
	~IconButton();

private:
	void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE; //�����¼�
	void resizeEvent(QResizeEvent *event);
	void enterEvent(QEvent *); //��������¼�
	void leaveEvent(QEvent *); //����Ƴ��¼�
	void mousePressEvent(QMouseEvent *event);   //��갴��
	void mouseReleaseEvent(QMouseEvent *event); //����ͷ�

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
	//����ͼ��
	void setIcon(QString iconPath) { m_icon = QPixmap(iconPath); }
	void setIcon(QPixmap icon) { m_icon = icon; }
	//����ͼ���������ƫ��
	void setIconOffsetXY(int x, int y) { m_offsetX = x; m_offsetY = y; }
	//����ͼ�����ſ��ƫ��
	void setIconOffsetWH(int w, int h) { m_offsetW = w; m_offsetH = h; }
	//����ͼ�꿹���
	void setIconRenderHint(bool b) { m_TransformationMode = b ? Qt::SmoothTransformation : Qt::FastTransformation; }
};
