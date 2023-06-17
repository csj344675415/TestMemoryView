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
	// 响应鼠标点击
	setSectionsClickable(true);
	// 监听鼠标移动事件
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
		//在第 NULL_COLUMN 列绘制复选按钮，NULL_COLUMN = -1 不会制
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

		//QSS样式绘制
		QCheckBox checkBox;
		style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter, &checkBox);
		//普通绘制
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
		//鼠标点击复选按钮所在列则改变状态
		if (m_bTristate && m_bNoChange){
			m_bChecked = true;		//如果当前状态是半选则改为选中
			m_bNoChange = false;
		}
		else{
			m_bChecked = !m_bChecked;	//否则改为未选中
		}

		Qt::CheckState state = m_bChecked ? Qt::Checked : Qt::Unchecked;
		emit signal_stateChanged(state);
		updateSection(0);
	}
	else {
		QHeaderView::mouseReleaseEvent(event);
	}
}

// 槽函数，用于更新复选框状态
void MyHeaderView::slot_stateChanged(int state)
{
	if (state == Qt::PartiallyChecked)
	{
		m_bTristate = true;	//设为半选
		m_bNoChange = true;	//内容无变化
	}
	else {
		m_bNoChange = false;
	}

	//如果表格全部选中则表头设置为选中，如果为全部未选中则表头设置为未选中
	m_bChecked = (state != Qt::Unchecked);
	updateSection(0);
}