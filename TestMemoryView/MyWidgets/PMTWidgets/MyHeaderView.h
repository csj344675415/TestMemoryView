#pragma once

#include <QHeaderView>

class MyHeaderView : public QHeaderView
{
	Q_OBJECT

public:
	MyHeaderView(Qt::Orientation orientation, QWidget *parent);
	~MyHeaderView();


	public slots:
	void slot_stateChanged(int state);
signals:
	void signal_stateChanged(int state);

protected:
	void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const;
	//void mousePressEvent(QMouseEvent * evente);
	void mouseReleaseEvent(QMouseEvent *event);

public:
	bool m_bChecked;	//复选框状态
	bool m_bTristate;	//半选状态
	bool m_bNoChange;	//内容是否有变化
};

