#include "MyAbstractTableModel.h"

MyAbstractTableModel::MyAbstractTableModel(QObject *parent, QSortFilterProxyModel *pProxyModel)
	: QAbstractTableModel(parent)
	, m_column(0)
	, m_row(0)
	, nSelectedCount(0)
	, pSortFilterProxyModel(pProxyModel)
{
	//构造Item数据模板	
	//m_tempItem.data = QVariant();
	//m_tempItem.icon = QIcon();
	m_tempItem.color = Qt::black;
	m_tempItem.alignment = (Qt::AlignLeft | Qt::AlignVCenter);
	m_tempItem.bChecked = false;
	m_tempItem.colorBackground = Qt::color0;
}

MyAbstractTableModel::~MyAbstractTableModel()
{
}

int MyAbstractTableModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_itemData.count();
}

int MyAbstractTableModel::columnCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent);
	return m_column;
}

bool MyAbstractTableModel::setData(const QModelIndex &index, const QVariant &value, int role)	
{
	if (!index.isValid())
		return false;

	int row = index.row();
	int column = index.column();

	switch (role)
	{
	case Qt::DisplayRole:
		m_itemData[row][column].data = value;
	case Qt::EditRole:
		break;
	case Qt::DecorationRole:
		if (column != 0) {
			m_itemData[row][column].icon = value.value<QIcon>();
		}
		break;
	case Qt::TextAlignmentRole:
		if (column != 0) {
			m_itemData[row][column].alignment = value.value<Qt::Alignment>();
		}
		break;
	case Qt::TextColorRole:
		if (column != 0) {
			m_itemData[row][column].color = value.value<QColor>();
		}
		break;
	case Qt::CheckStateRole:
	case Qt::UserRole:
		if (column == 0)
		{
			m_itemData[row][column].bChecked = value.toBool();
			stateChanged(value.toBool());
		}
		break;
	default:
		return false;
	}

	emit dataChanged(index, index);
	return true;
}

QVariant MyAbstractTableModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return QVariant();

	int row = index.row();
	int column = index.column();

	//读取的时候用 at() 比用 [下标] 快
	switch (role)
	{
	case Qt::DisplayRole:		//显示	
	case Qt::EditRole:			//编辑
		////过滤第4列不显示
		//if (column != 4) {
		//	return m_itemData.at(row).at(column).data;
		//}
		return m_itemData.at(row).at(column).data;
		break;
	case Qt::DecorationRole:	//图标
		return m_itemData.at(row).at(column).icon;
		break;
	case Qt::ToolTipRole:	//提示标签
		return m_itemData.at(row).at(column).strToolTip;
		break;
	case Qt::FontRole:	//字体
		return m_itemData.at(row).at(column).font;
		break;
	case Qt::TextAlignmentRole:	//对齐方式	
		return QVariant::fromValue(m_itemData.at(row).at(column).alignment);
		break;
	case Qt::BackgroundRole:	//背景色
		if (m_bAlternatingRowColors && row%2 == 0){
			//隔行换色
			return QColor(240,240,240);
		}
		return m_itemData.at(row).at(column).colorBackground;
		break;
	case Qt::ForegroundRole:	//前景色(文本颜色)
		return m_itemData.at(row).at(column).colorForeground;
		break;
	//用户自定义数据	
	case Qt::UserRole:				
		//若要在 MyStyledItemDelegate 的绘图事件中使用该数据则在这里返回，放在 UserRole +1 里返回也行
		if (column == 0) {
			return m_itemData.at(row).at(0).bChecked;
		}
		if (column == 4) {
			return m_itemData.at(row).at(column).data;
		}
		break;
	case Qt::UserRole + 1:
		return m_itemData.at(row).at(column).userData;
		break;
	default:
		break;
	}

	return QVariant();
}

QVariant MyAbstractTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (orientation == Qt::Horizontal)
	{
		switch (role)
		{
		case Qt::DisplayRole:
			return m_headerData[section].headerData;
		case Qt::DecorationRole:
			return QIcon(m_headerData[section].icon);
		case Qt::TextAlignmentRole:
			if (!m_headerData[section].alignment) {
				//return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
				return QVariant(Qt::AlignCenter);
			}
			return QVariant::fromValue(m_headerData[section].alignment);
		case Qt::TextColorRole:
			return m_headerData[section].color;
		default:
			return QVariant();
		}
	}

	////获取排序后的索引
	//QModelIndex temp = pQSortFilterProxyModel->index(section, 0);
	////转为排序前的索引
	//QModelIndex index = pQSortFilterProxyModel->mapToSource(temp);

	if (orientation == Qt::Vertical)
	{
		switch (role)
		{
		case Qt::DisplayRole: 
			return section;
		default:
			return QVariant();
		}
	}

	return QVariant();
}

Qt::ItemFlags MyAbstractTableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return QAbstractItemModel::flags(index);

	//可启用，可选中,可编辑
	Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
	//只允许前3列编辑
	//if (index.column() == 0 || index.column() == 1 || index.column() == 2) {
	//	flags |= Qt::ItemIsEditable;
	//}
	flags |= Qt::ItemIsEditable;

	return flags;
}

void MyAbstractTableModel::updateData()
{
	beginResetModel();
	endResetModel();
}

void MyAbstractTableModel::setSortFilterProxy(QSortFilterProxyModel *p)
{
	pSortFilterProxyModel = p;
}

QModelIndex MyAbstractTableModel::getModelIndex(int row, int column)
{
	if (!pSortFilterProxyModel) {
		return QModelIndex();
	}
	//获取排序后的索引
	QModelIndex temp = pSortFilterProxyModel->index(row, column);
	//转为排序前的索引
	QModelIndex index = pSortFilterProxyModel->mapToSource(temp);
	//判断转换后的索引是否有效
	if (!index.isValid()) {
		return this->index(row, column);
	}
	return index;
}

void MyAbstractTableModel::setRowCount(int rows)
{
	m_row = rows;

	// 允许外部设置行数的话则将 下面的代码 取消注释
	//int row = m_itemData.count();
	//for (int i = row; i < m_row; i++)
	//{
	//	m_itemData.append(QList<ITEM_DATA>());
	//	int col = m_itemData.at(i).count();
	//	for (int j = col; j < m_column; j++)
	//	{
	//		m_itemData[i].append(m_tempItem);
	//	}
	//}
}
void MyAbstractTableModel::setColumnCount(int columns)
{
	m_column = columns;
	//
	int colNum = m_headerData.count();
	for (int j = colNum; j < m_column; j++) {
		m_headerData.append(HEADER_DATA());
	}

	// 允许外部设置列数的话则将下面的代码取消注释
	//int rowNum = m_itemData.count();
	//if (rowNum) {
	//	for (int i = 0; i < rowNum; i++)
	//	{
	//		int colNum2 = m_itemData.at(i).count();
	//		for (int j = colNum2; j < m_column; j++)
	//		{
	//			m_itemData[i].append(m_tempItem);
	//		}
	//	}
	//}
}

void MyAbstractTableModel::setHeaderData(int columns, QString text)
{
	if (columns >= m_column) {
		setColumnCount(columns + 1);
	}
	m_headerData[columns].headerData = text;
}
void MyAbstractTableModel::setHeaderData(int columns, Qt::Alignment alignment)
{
	if (columns >= m_column) {
		setColumnCount(columns + 1);
	}
	m_headerData[columns].alignment = alignment;
}
void MyAbstractTableModel::setHeaderData(int columns, QColor color, Qt::Alignment alignment)
{
	if (columns >= m_column) {
		setColumnCount(columns + 1);
	}
	m_headerData[columns].color = color;
	m_headerData[columns].alignment = alignment;
}
void MyAbstractTableModel::setHeaderData(int columns, QString text, QColor color, Qt::Alignment alignment)
{
	if (columns >= m_column) {
		setColumnCount(columns + 1);
	}
	m_headerData[columns].headerData = text;
	m_headerData[columns].color = color;
	m_headerData[columns].alignment = alignment;
}


bool MyAbstractTableModel::addItem(int &row, int &column)
{
	if (row < 0 || column < 0) {
		return false;
	}

	//添加数据
	if (row - m_row == 0) {
		setRowCount(row + 1);
		if (column <= m_column) {
			m_itemData.append(QList<ITEM_DATA>());
			int num = m_itemData.at(row).count();
			for (int i = num; i < m_column; i++) {
				//添加一个Item数据模板
				m_itemData[row].append(m_tempItem);
			}
		}
		else {
			return false;
		}
	}
	else if (row - m_row > -1) {	//小于等于-1视为修改数据，否则传入的是无效行
		return false;
	}

	//如果传入的行大于表格最大行数，则扩充（row - m_row）行
	//if (row >= m_row) {
	//	setRowCount(row + 1);
	//	int num = m_itemData.count();
	//	for (int i = num; i < m_row; i++) {
	//		//m_itemData容量+1
	//		m_itemData.append(QList<ITEM_DATA>());
	//	}
	//}
	//else {
	//	int num = m_itemData.count();
	//	for (int i = num; i < m_row; i++) {
	//		//m_itemData容量+1
	//		m_itemData.append(QList<ITEM_DATA>());
	//	}
	//}

	//如果传入的列数大于表格最大列数，则扩充（col - m_column）列
	//if (column >= m_column) {
	//	setColumnCount(column + 1);
	//	int num = m_itemData.at(row).count();
	//	for (int i = num; i < m_column; i++) {
	//		//添加一个Item数据模板
	//		m_itemData[row].append(m_tempItem);
	//	}
	//}
	//else {
	//	int num = m_itemData.at(row).count();
	//	for (int i = num; i < m_column; i++) {
	//		//添加一个Item数据模板	
	//		m_itemData[row].append(m_tempItem);
	//	}
	//}

	QModelIndex index = getModelIndex(row, column);
	row = index.row();
	column = index.column();

	return true;
}
void MyAbstractTableModel::setIcon(int row, int column, QIcon icon)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].icon = icon;
}
void MyAbstractTableModel::setItemData(int row, int column, QVariant data)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].data = data;
}
void MyAbstractTableModel::setTextColor(int row, int column, QColor color)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].colorForeground = color;
}
void MyAbstractTableModel::setTextAlignment(int row, int column, Qt::Alignment alignment)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].alignment = alignment;
}
void MyAbstractTableModel::setToolTip(int row, int column, QVariant data)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].strToolTip = data;
}
void MyAbstractTableModel::setFont(int row, int column, QFont font)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].font = font;
}
void MyAbstractTableModel::setBackgroundColor(int row, int column, QColor color)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].colorBackground = color;
}
void MyAbstractTableModel::setForegroundColor(int row, int column, QColor color)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].colorForeground = color;
}
void MyAbstractTableModel::setItemData(int row, int column, QIcon icon, QVariant data)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].icon = icon;
	m_itemData[row][column].data = data;
}
void MyAbstractTableModel::setItemData(int row, int column, QColor color, QVariant data)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].colorForeground = color;
	m_itemData[row][column].data = data;
}

void MyAbstractTableModel::setUserData(int row, int column, QVariant data)
{
	if (!addItem(row, column)) {
		return;
	}
	m_itemData[row][column].userData = data;
}

void MyAbstractTableModel::setAlternatingRowColors(bool bl, QColor color)
{ 
	m_bAlternatingRowColors = bl; 
	m_rowColor = color;
}
void MyAbstractTableModel::insertItemData(int row, int column, QVariant data)
{
	if (row < 0 || row > m_row || column < 0 || column > m_column) {
		return;
	}

	m_row++;

	//转换为数据源索引
	QModelIndex nIndex = getModelIndex(row, column);
	int nRow = nIndex.row();
	int nCol = nIndex.column();

	m_itemData.insert(nRow, QList<ITEM_DATA>());
	for (int i = 0; i < m_column; i++) {
		m_itemData[nRow].append(m_tempItem);
	}
	m_itemData[nRow][nCol].data = data;

	updateData();
}

int MyAbstractTableModel::getRows()
{
	return m_row;
}
int MyAbstractTableModel::getColumns()
{
	return m_column;
}

void MyAbstractTableModel::removeRow(int row, bool bl)
{
	QModelIndex index = getModelIndex(row, 0);
	int nRow = index.row();
	if (nRow < 0) {
		return;
	}
	if (bl) {
		for (int i = 0; i < m_column; i++) {
			m_itemData[nRow][i] = m_tempItem;
		}
	}
	else {
		m_itemData.removeAt(nRow);
		m_row--;
	}

	updateData();
}
void MyAbstractTableModel::removeColumn(int column, bool bl)
{
	if (column < 0) {
		return;
	}
	if (bl) {
		if (column < m_column) {
			for (int i = 0; i < m_row; i++) {
				m_itemData[i][column] = m_tempItem;
			}
			m_headerData[column] = HEADER_DATA();
		}
	}
	else {
		if (column < m_column) {
			for (int i = 0; i < m_row; i++) {
				m_itemData[i].removeAt(column);
			}
			m_headerData.removeAt(column);
			m_column--;
		}
	}

	updateData();
}
void MyAbstractTableModel::clearRows(bool bl)
{
	if (bl) {
		for (int i = 0; i < m_row; i++) {
			removeRow(i, bl);
		}
	}
	else {
		m_itemData.clear();
		m_row = 0;
	}
	updateData();
}

void MyAbstractTableModel::clear(bool bl)
{
	if (bl) {
		for (int i = 0; i < m_column; i++) {
			m_headerData[i] = HEADER_DATA();
		}

		for (int j = 0; j < m_row; j++) {
			removeRow(j, true);
		}
	}
	else {
		m_headerData.clear();
		m_itemData.clear();
		m_row = 0;
		m_column = 0;
	}

	updateData();
}

// 复选状态改变，通知表头更新复选状态
void MyAbstractTableModel::stateChanged(bool Selected)
{
	Qt::CheckState state;
	int nCount = m_itemData.count();	//获取总行数

	if (Selected) {
		nSelectedCount++;
	}
	else {
		nSelectedCount--;
	}

	if (nSelectedCount >= nCount)
	{
		state = Qt::Checked;			//选中的数量大于等于总数，则表头设置为选中状态
	}
	else if (nSelectedCount > 0)
	{
		state = Qt::PartiallyChecked;	//否则只要有一个以上选中则设置表头为半选状态
	}
	else if (nSelectedCount == 0)
	{
		state = Qt::Unchecked;			//否则表头设置为未选中
	}

	emit signal_stateChanged(state);
}

// 接收表头复选状态更新变化，修改表格复选状态
void MyAbstractTableModel::slot_StateChanged(int state)
{
	for (int i = 0; i < m_row; ++i)
	{
		m_itemData[i][0].bChecked = state;
	}

	if (state) {
		nSelectedCount = m_row;
	}
	else {
		nSelectedCount = 0;
	}
	updateData();
}

