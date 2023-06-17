#pragma once

#include <QTableView>
#include <QStandardItemModel>
#include <QStandardItem>


#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma warning(disable:4091 4311)

class QMenu;
class InfoViewItemModel;

class InfoViewBase : public QTableView
{
	Q_OBJECT

public:
	InfoViewBase(QWidget *parent = Q_NULLPTR);
	~InfoViewBase();

protected: //受保护
	//virtual void dragEnterEvent(QDragEnterEvent *event);
	//virtual void dropEvent(QDropEvent *event);

public slots:
	virtual void slot_customContextMenuRequested(const QPoint & pos);
	virtual void slot_clickedView(const QModelIndex & index);
	virtual void slot_doubleView(const QModelIndex & index);

	virtual void slot_update();
	//virtual void slot_openDir();
	virtual void slot_filter();
	virtual void slot_copy();

signals:
	void signal_selectItemValue(QString &strValue);
	void signal_clickedView(const QModelIndex & index);	//点击进程列表信号
	void signal_doubleView(const QModelIndex & index);	//双击进程列表信号
	void signal_updateView();
public:
	//------------------------ 外部调用接口 -----------------------------------------//
	//QModelIndex getIndex();
	void showInfoView(unsigned int pid = 0);


protected: //受保护
	bool EnablePrivilege();
	virtual void initView();
	virtual void initMenu() {};
	virtual void enmuInfo(unsigned int pid) = 0;
	virtual void updateView(unsigned int pid = 0) = 0;
	virtual void readSettings() {}
	virtual void writeSettings() {}
	bool is64OS();
	bool is64Process(unsigned int dwProcessID);

	QMenu *_pMenu = nullptr;
	InfoViewItemModel* _pModel;
	QMap<int, QVariant> _hideData;
	int _row;
	int _col;
	bool _bFilter;
	unsigned int _pid;
};


class InfoViewItemModel : public QStandardItemModel
{
	Q_OBJECT

public:
	InfoViewItemModel(QObject *parent) :QStandardItemModel(parent) {}
	~InfoViewItemModel() {}

public:
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) {
		if (!index.isValid())
			return false;

		int row = index.row();
		int column = index.column();

		switch (role)
		{
		case Qt::EditRole:
			return true;
		}
		return QStandardItemModel::setData(index, value, role);
	}

	//QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) {
	//	if (!index.isValid())
	//		return false;

	//	int row = index.row();
	//	int column = index.column();

	//	switch (role)
	//	{
	//	case Qt::UserRole + 1:
	//		return true;
	//	}
	//	return QStandardItemModel::data(index, role);
	//}
};

