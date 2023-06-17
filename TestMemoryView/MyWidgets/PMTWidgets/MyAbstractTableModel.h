#pragma once
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include "itemdef.h"

/*
注意事项：必须传入一个 QSortFilterProxyModel 对象，否则获取索引将失败

*/

class MyAbstractTableModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	MyAbstractTableModel(QObject *parent, QSortFilterProxyModel *pProxyModel);
	~MyAbstractTableModel();

public:
	int rowCount(const QModelIndex &parent) const;
	int columnCount(const QModelIndex &parent) const;
	bool setData(const QModelIndex &index, const QVariant &value, int role);
	QVariant data(const QModelIndex &index, int role) const;
	QVariant headerData(int section, Qt::Orientation orientation, int role) const;
	Qt::ItemFlags flags(const QModelIndex &index) const;


signals:
	void signal_stateChanged(int state);
	private slots:
	void slot_StateChanged(int state);

private:
	// 私有成员，不允许外部调用
	/*
	该类型结构体声明在 itemdef.h 头文件里
	HEADER_DATA
	ITEM_DATA
	*/
	QList<HEADER_DATA> m_headerData;	//存放头部数据
	ITEM_DATA m_tempItem;				//Item数据模板
	QList<QList<ITEM_DATA>> m_itemData;	//存放Item数据
	int nSelectedCount;					//勾选计数

	// 行列数
	int m_row, m_column;

	// 开启隔行换色
	bool m_bAlternatingRowColors = false;
	QColor m_rowColor = QColor(240, 240, 240);

	// 设置行数
	void setRowCount(int rows);

	// 设置列数
	void setColumnCount(int columns);

	// 更改复选框状态
	void stateChanged(bool Selected);

	// 添加Item
	bool addItem(int &row, int &column);								

public:
	//公有成员，外部调用接口

	// 更新数据
	void updateData();

	// 设置表头数据
	void setHeaderData(int columns, QString text);											//标题
	void setHeaderData(int columns, Qt::Alignment alignment);								//对齐方式
	void setHeaderData(int columns, QColor color, Qt::Alignment alignment);					//字体颜色，对齐方式
	void setHeaderData(int columns, QString text, QColor color, Qt::Alignment alignment);	//标题，字体颜色，对齐方式
	
	/*
	设置UserData和取出UserData的方法：
	内部提前设置号角色：
	在data(...)里同样添加一个 Qt::UserRole + 1 的角色，用于将数据取出，如：m_itemData.at(row).at(column).userData;

	外部添加数据和取出数据：
	添加：setUserData(row, col, 998);	col的值随便填，不要超过设置的列数即可
	取出：getModelIndex(row, col).data(Qt::UserRole + 1).toInt();

	*/
	// 设置表格Item数据
	void setItemData(int row, int column, QVariant data);				//设置|修改Item数据	数据
	void setItemData(int row, int column, QIcon icon, QVariant data);	//设置|修改Item数据	图标+数据
	void setItemData(int row, int column, QColor color, QVariant data);	//设置|修改Item数据	颜色+数据
	void setIcon(int row, int column, QIcon icon);						//设置|修改Item数据	图标
	void setTextColor(int row, int column, QColor color);				//设置|修改Item数据	文本颜色
	void setTextAlignment(int row, int column, Qt::Alignment alignment);//设置|修改Item数据	文本对齐方式
	void setToolTip(int row, int column, QVariant data);				//设置|修改Item数据	鼠标停留在Item上时弹出的提示标签
	void setFont(int row, int column, QFont font);						//设置|修改Item数据	文本字体
	void setBackgroundColor(int row, int column, QColor color);			//设置|修改Item数据	Item背景色
	void setForegroundColor(int row, int column, QColor color);			//设置|修改Item数据	Item前景色（文本颜色）
	void setUserData(int row, int column, QVariant data);				//设置用户数据	该数据不显示在视图上
	
	void setAlternatingRowColors(bool bl, QColor color = QColor(240, 240, 240));	//设置隔行换色

	// 插入数据（也可用于修改数据） 
	// 如果插入的位置不对，那应该是插入前有过排序动作，插入后View更新，会自动重新排序，将插入的数据排到正确的位置，导致看起来位置不对，实际上在数据源里的位置是正确的
	void insertItemData(int row, int column, QVariant data); //设置|修改Item数据，

	// 代理排序模型的指针
	QSortFilterProxyModel *pSortFilterProxyModel = NULL;
	// 设置代理排序模型指针
	void setSortFilterProxy(QSortFilterProxyModel *p);
	// 获取排序前的数据源索引
	QModelIndex getModelIndex(int row, int column);	

	// 删除、清空操作：参数 （bl = true） 则表示保留表头，只清除Iten数据。默认为false不保留表头
	void removeRow(int row, bool bl = false);		//删除行
	void removeColumn(int column, bool bl = false);	//删除列
	void clearRows(bool bl = false);				//清空所有行
	void clear(bool bl = false);					//清空表格
	int getRows();									//获取行数
	int getColumns();								//获取列数
};

/*
Item主要角色及其描述

常量                     描述
Qt::DisplayRole       显示文字
Qt::DecorationRole    绘制装饰数据（通常是图标）
Qt::EditRole          在编辑器中编辑的数据
Qt::ToolTipRole       工具提示
Qt::StatusTipRole     状态栏提示
Qt::WhatsThisRole     What's This文字
Qt::SizeHintRole      尺寸提示
Qt::FontRole          默认代理的绘制使用的字体
Qt::TextAlignmentRole 默认代理的对齐方式
Qt::BackgroundRole    默认代理的背景画刷
Qt::ForegroundRole    默认代理的前景画刷
Qt::CheckStateRole    默认代理的检查框状态
Qt::UserRole          用户自定义的数据的起始位置

*/
