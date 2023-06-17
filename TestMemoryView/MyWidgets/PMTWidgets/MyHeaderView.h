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
	bool m_bChecked;	//��ѡ��״̬
	bool m_bTristate;	//��ѡ״̬
	bool m_bNoChange;	//�����Ƿ��б仯
};

