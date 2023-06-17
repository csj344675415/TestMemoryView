#include "MyHeaderView.h"
#include <QMouseEvent>
#include <QPainter>
#include <QCheckBox>

#include "itemdef.h"

MyHeaderView::MyHeaderView(Qt::Orientation orientation, QWidget *parent)
	: QHeaderView(orientation, parent)
	, m_bChecked(false)
	, m_bTristate(false)
	, m_bNoChange(false)
{
	// ��Ӧ�����
	setSectionsClickable(true);
	// ��������ƶ��¼�
	//setMouseTracking(true);	
}

MyHeaderView::~MyHeaderView()
{
}

void MyHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
	painter->save();
	QHeaderView::paintSection(painter, rect, logicalIndex);
	painter->restore();

	if (logicalIndex == NULL_COLUMN)
	{
		//�ڵ� NULL_COLUMN �л��Ƹ�ѡ��ť��NULL_COLUMN = -1 ������
		QStyleOptionButton option;
		option.initFrom(this);

		if (m_bTristate && m_bNoChange)
			option.state |= QStyle::State_NoChange;
		else
			option.state |= m_bChecked ? QStyle::State_On : QStyle::State_Off;

		//option.iconSize = QSize(20, 20);
		int x = rect.x() + rect.width() / 2 - 15 / 2;
		int y = rect.y() + rect.height() / 2 - 15 / 2;
		int w = 15;
		int h = 15;
		option.rect = QRect(x, y, w, h);

		//QSS��ʽ����
		QCheckBox checkBox;
		style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter, &checkBox);
		//��ͨ����
		//style()->drawControl(QStyle::CE_CheckBox, &option, painter);
	}
}

//void MyHeaderView::mousePressEvent(QMouseEvent * event)
//{
//	QHeaderView::mousePressEvent(event);
//}

void MyHeaderView::mouseReleaseEvent(QMouseEvent *event)
{
	int nColumn = logicalIndexAt(event->pos());
	if ((event->button() == Qt::LeftButton) && (nColumn == NULL_COLUMN))
	{
		//�������ѡ��ť��������ı�״̬
		if (m_bTristate && m_bNoChange){
			m_bChecked = true;		//�����ǰ״̬�ǰ�ѡ���Ϊѡ��
			m_bNoChange = false;
		}
		else{
			m_bChecked = !m_bChecked;	//�����Ϊδѡ��
		}

		Qt::CheckState state = m_bChecked ? Qt::Checked : Qt::Unchecked;
		emit signal_stateChanged(state);
		updateSection(0);
	}
	else {
		QHeaderView::mouseReleaseEvent(event);
	}
}

// �ۺ��������ڸ��¸�ѡ��״̬
void MyHeaderView::slot_stateChanged(int state)
{
	if (state == Qt::PartiallyChecked)
	{
		m_bTristate = true;	//��Ϊ��ѡ
		m_bNoChange = true;	//�����ޱ仯
	}
	else {
		m_bNoChange = false;
	}

	//������ȫ��ѡ�����ͷ����Ϊѡ�У����Ϊȫ��δѡ�����ͷ����Ϊδѡ��
	m_bChecked = (state != Qt::Unchecked);
	updateSection(0);
}