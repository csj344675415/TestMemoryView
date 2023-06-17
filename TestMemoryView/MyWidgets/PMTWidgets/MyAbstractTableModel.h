#pragma once
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include "itemdef.h"

/*
ע��������봫��һ�� QSortFilterProxyModel ���󣬷����ȡ������ʧ��

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
	// ˽�г�Ա���������ⲿ����
	/*
	�����ͽṹ�������� itemdef.h ͷ�ļ���
	HEADER_DATA
	ITEM_DATA
	*/
	QList<HEADER_DATA> m_headerData;	//���ͷ������
	ITEM_DATA m_tempItem;				//Item����ģ��
	QList<QList<ITEM_DATA>> m_itemData;	//���Item����
	int nSelectedCount;					//��ѡ����

	// ������
	int m_row, m_column;

	// �������л�ɫ
	bool m_bAlternatingRowColors = false;
	QColor m_rowColor = QColor(240, 240, 240);

	// ��������
	void setRowCount(int rows);

	// ��������
	void setColumnCount(int columns);

	// ���ĸ�ѡ��״̬
	void stateChanged(bool Selected);

	// ���Item
	bool addItem(int &row, int &column);								

public:
	//���г�Ա���ⲿ���ýӿ�

	// ��������
	void updateData();

	// ���ñ�ͷ����
	void setHeaderData(int columns, QString text);											//����
	void setHeaderData(int columns, Qt::Alignment alignment);								//���뷽ʽ
	void setHeaderData(int columns, QColor color, Qt::Alignment alignment);					//������ɫ�����뷽ʽ
	void setHeaderData(int columns, QString text, QColor color, Qt::Alignment alignment);	//���⣬������ɫ�����뷽ʽ
	
	/*
	����UserData��ȡ��UserData�ķ�����
	�ڲ���ǰ���úŽ�ɫ��
	��data(...)��ͬ�����һ�� Qt::UserRole + 1 �Ľ�ɫ�����ڽ�����ȡ�����磺m_itemData.at(row).at(column).userData;

	�ⲿ������ݺ�ȡ�����ݣ�
	��ӣ�setUserData(row, col, 998);	col��ֵ������Ҫ�������õ���������
	ȡ����getModelIndex(row, col).data(Qt::UserRole + 1).toInt();

	*/
	// ���ñ��Item����
	void setItemData(int row, int column, QVariant data);				//����|�޸�Item����	����
	void setItemData(int row, int column, QIcon icon, QVariant data);	//����|�޸�Item����	ͼ��+����
	void setItemData(int row, int column, QColor color, QVariant data);	//����|�޸�Item����	��ɫ+����
	void setIcon(int row, int column, QIcon icon);						//����|�޸�Item����	ͼ��
	void setTextColor(int row, int column, QColor color);				//����|�޸�Item����	�ı���ɫ
	void setTextAlignment(int row, int column, Qt::Alignment alignment);//����|�޸�Item����	�ı����뷽ʽ
	void setToolTip(int row, int column, QVariant data);				//����|�޸�Item����	���ͣ����Item��ʱ��������ʾ��ǩ
	void setFont(int row, int column, QFont font);						//����|�޸�Item����	�ı�����
	void setBackgroundColor(int row, int column, QColor color);			//����|�޸�Item����	Item����ɫ
	void setForegroundColor(int row, int column, QColor color);			//����|�޸�Item����	Itemǰ��ɫ���ı���ɫ��
	void setUserData(int row, int column, QVariant data);				//�����û�����	�����ݲ���ʾ����ͼ��
	
	void setAlternatingRowColors(bool bl, QColor color = QColor(240, 240, 240));	//���ø��л�ɫ

	// �������ݣ�Ҳ�������޸����ݣ� 
	// ��������λ�ò��ԣ���Ӧ���ǲ���ǰ�й��������������View���£����Զ��������򣬽�����������ŵ���ȷ��λ�ã����¿�����λ�ò��ԣ�ʵ����������Դ���λ������ȷ��
	void insertItemData(int row, int column, QVariant data); //����|�޸�Item���ݣ�

	// ��������ģ�͵�ָ��
	QSortFilterProxyModel *pSortFilterProxyModel = NULL;
	// ���ô�������ģ��ָ��
	void setSortFilterProxy(QSortFilterProxyModel *p);
	// ��ȡ����ǰ������Դ����
	QModelIndex getModelIndex(int row, int column);	

	// ɾ������ղ��������� ��bl = true�� ���ʾ������ͷ��ֻ���Iten���ݡ�Ĭ��Ϊfalse��������ͷ
	void removeRow(int row, bool bl = false);		//ɾ����
	void removeColumn(int column, bool bl = false);	//ɾ����
	void clearRows(bool bl = false);				//���������
	void clear(bool bl = false);					//��ձ��
	int getRows();									//��ȡ����
	int getColumns();								//��ȡ����
};

/*
Item��Ҫ��ɫ��������

����                     ����
Qt::DisplayRole       ��ʾ����
Qt::DecorationRole    ����װ�����ݣ�ͨ����ͼ�꣩
Qt::EditRole          �ڱ༭���б༭������
Qt::ToolTipRole       ������ʾ
Qt::StatusTipRole     ״̬����ʾ
Qt::WhatsThisRole     What's This����
Qt::SizeHintRole      �ߴ���ʾ
Qt::FontRole          Ĭ�ϴ���Ļ���ʹ�õ�����
Qt::TextAlignmentRole Ĭ�ϴ���Ķ��뷽ʽ
Qt::BackgroundRole    Ĭ�ϴ���ı�����ˢ
Qt::ForegroundRole    Ĭ�ϴ����ǰ����ˢ
Qt::CheckStateRole    Ĭ�ϴ���ļ���״̬
Qt::UserRole          �û��Զ�������ݵ���ʼλ��

*/
