#pragma once

#include <QAbstractScrollArea>
#include "ui_HexEdit.h"
#include <QStack>

#include <thread>


#include "EditorWidgets.h"


class QPaintEvent;
class QMenu;
class QTimerEvent;
class QUndoStack;


enum dataType {
	e1ByteHex = 0,
	e1ByteDec,
	e2ByteHex,
	e2ByteDec,
	e4ByteHex,
	e4ByteDec,
	e8ByteHex,
	e8ByteDec,
	e4ByteFloat,
	e8ByteDouble,
};

enum ascType {
	eASCII = 0,
	eUTF8,
	eUTF16,
};

enum eWidthId {
	eAddr_Width = 0,
	eData_Width,
	eText_Width,
};

typedef struct MEM_INFO
{
	unsigned long long allocationBase;	//�����ڴ�ҳ��Ӧ��ģ���ַ
	unsigned long long beginAddr;		//��ǰҳ�Ŀ�ʼ��ַ
	unsigned long long endAddr;			//��ǰҳ�Ľ�����ַ
	unsigned int size;					//��ǰҳ�Ĵ�С
	unsigned int state;					//�ڴ�״̬
	unsigned int state2;					//�ڴ�״̬
	unsigned int type;					//�ڴ�����
	unsigned int protect;				//��ǰҳ�ı�������
	QString strProtect;					//ҳ���������ı�
	QColor color;
	QColor color2;
	bool isBase;						//�ǻ�ַ��Χ
	QString moduleName;
};

typedef struct HexEditItem
{
	QRect rect;
	QString text;
};

typedef struct RENDER_DATA_INFO {
	int ms;				//Ⱦɫʣ��ʱ��
	unsigned char diaphaneity;	//͸����
};

class HexEdit : public QAbstractScrollArea
{
	Q_OBJECT

public:
	HexEdit(QWidget *parent = Q_NULLPTR);
	~HexEdit();

private:
	Ui::HexEdit ui;

protected:
	// Handle events
	void timerEvent(QTimerEvent *event); //����ʱ��
	//void keyPressEvent(QKeyEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void mouseDoubleClickEvent(QMouseEvent *event); //���˫���¼�
	void paintEvent(QPaintEvent * event);
	void resizeEvent(QResizeEvent * event);
	virtual void wheelEvent(QWheelEvent * event);
	//virtual bool focusNextPrevChild(bool next);

signals:
	void signal_updateStatusBar(DataInfo info);
	void signal_DoubleData(DataInfo info, int dataType);
	void signal_updateHexEditView();
public slots:
void slot_sliderPressed();
void slot_sliderReleased();
void slot_actionTriggered(int value);

void slot_changeData(DataInfo info, int size);
void slot_gotoAddr(unsigned long long addr);
void slot_setTime(int ms);

//MENU
void slot_dataType();
void slot_textType();
void slot_protectType();
void slot_divisionType();

void slot_symbol();
void slot_toAddr();
void slot_changeTime();

void slot_copy();
void slot_paste();

void slot_undo();
void slot_redo();

void slot_backspaceAddr();



public:
	void setData(unsigned int pid, unsigned long long addr = -1);
	void updateStop();	//ֹͣ����ˢ��
	void updateStart();	//��ʼ����ˢ��
	void changeAddr(unsigned long long addr);
	void updeteView();

protected:
	void init();
	void initMenu();
	QMenu* _pMenu = nullptr;
	QVector<QAction*> _vecAction_DataType;		//��������
	QVector<QAction*> _vecAction_TextType;		//�ı�����
	QVector<QAction*> _vecAction_divisionType;	//�ָ��
	QVector<QAction*> _vecAction_ProtectType;	//ҳ�汣������

	void _updateViewItem();
	QFontMetrics* _pFontMetrics;
	int _rowHeight = 0;			//һ�еĸ߶�
	int _maxRows = 0;			//��ͼ����ʾ���������
	int _maxAddrWidth = 0;		//��ַ�ַ����Ŀ��
	int _maxDataWidth = 0;		//�����ַ����Ŀ��
	int _maxTextWidth = 0;		//�ı��ַ����Ŀ��
	int _dataCols = 0;			//��ͼ����ʾ���ݵ�����
	int _textCols = 0;			//��ͼ����ʾ�ı����ݵ�����
	int _rtAddrItemWidth = 0;	//��ַItem�Ŀ��
	int _rtDataItemWidth = 0;	//����Item�Ŀ��
	int _rtTextItemWidth = 0;	//�ı�Item�Ŀ��
	unsigned int _maxBytes = 1024;	//��ͼ����ʾ������ֽ���


	int _getStringWidth(int id);	//��ȡ�ַ������
	int getDataItemWidth();			//��ȡ��������һ��Item�Ŀ��


	QRect _rtMemInfoArea;	//�ڴ���Ϣ����
	QRect _rtOffsetArea;	//��ַƫ������
	QRect _rtAddrArea;		//��ַ����
	QRect _rtDataArea;		//��������
	QRect _rtTextArea;		//�ı�����
	QRect _rtClickedArea;	//�����Item����

	HexEditItem _ProtectItem;				//��������
	HexEditItem _AllocationBaseAddrItem;	//��ǰ�ڴ��������ģ���ַ
	HexEditItem _BaseAddrItem;				//�ڴ���ַ
	HexEditItem _AddrSizeItem;				//�ڴ���С
	HexEditItem _ModuleNameItem;			//ģ����
	HexEditItem _AddrHeadItem;				//
	QVector<HexEditItem> _vecAddrItem;
	QVector<HexEditItem> _vecDataItem;
	QVector<HexEditItem> _vecTextItem;


	unsigned int _pid = 0;			//����PID
	unsigned char _pData[1024];		//���ݻ�����
	




	unsigned long long _firstAddr = -1;	//��һ�����ݵ�ʵ�ʵ�ַ
	int _divisionByte = 4;				//��ʼ4�ֽڷָ�


	bool _bPress = false;	//����Ƿ���

	inline void setBeginPos(QPoint &pos);
	inline void setEndPos(QPoint &pos);

	const HexEditItem *_pLastMouseItem = nullptr;	//��һ�����ָ���Item����ǰ���ָ���Item��_lastMouseItem����ͬʱ���Ż����ˢ��
	bool _bClicked = false;

	unsigned long long _dataAddr1 = 0;		//��갴��ʱ��ָ���Item���ŵ����ݶ�Ӧ�ĵ�ַ
	unsigned long long _dataAddr2 = 0;		//����ƶ�����ʱ��ָ���Item���ŵ����ݶ�Ӧ�ĵ�ַ
	unsigned long long _headAddr1 = 0;		//��갴��ʱ��ָ���Item�����е�ͷ��ַ
	unsigned long long _headAddr2 = 0;		//����ƶ�����ʱ��ָ���Item�����е�ͷ��ַ
	unsigned int _dataIndex1 = 0;			//��갴��ʱ��ָ���Item���ŵ�����λ�ڻ������������
	unsigned int _dataIndex2 = 0;			//����ƶ�����ʱ��ָ���Item���ŵ�����λ�ڻ������������
	unsigned int _itemIndex1 = 0;
	unsigned int _itemIndex2 = 0;
	unsigned int _colIndex = 0;				//�����ʱ��ָ���Item��Ӧ��������
	//bool _bSwap = false;					//�Ƿ񽻻���ַ���� _dataAddr2 С�� _dataAddr1 ʱ��Ҫ����
	int _releaseArea = 0;					//��굯��ʱλ���ĸ������������� = 1��λ���ı����� = 2���������� = 0


	int _dataType = e1ByteHex;	//��ǰ��ʾ����������
	int _textType = eASCII;		//��ǰ��ʾ���ı�����
	bool _bSymbol = false;		//�Ƿ���ʾ���ţ�10����ʱ�Ƿ���ʾ ��-�� �ţ�

	QString getHexData(unsigned long long addr, long long index);	//ȡ�ö�Ӧ�������͵��ַ���
	unsigned int getTextData(unsigned long long addr, long long index, int &itemIndex, QString &text);	//ȡ�ö�Ӧ�������͵��ַ���


	QColor _penColor;		//������ɫ
	QFont _originalFont;	//ԭʼ����
	QFont _boldFont;		//�Ӵ�����
	void drawMemoryInfo(QPainter &painter);					//�����ڴ���Ϣ
	void drawAddress(QPainter &painter);	//���Ƶ�ַ
	void drawData(QPainter &painter);		//��������
	void drawTextData(QPainter &painter);	//�����ı�����
	void drawDataHead(QPainter &painter);		//�������������ͷ
	void drawTextDataHead(QPainter &painter);	//�����ı������ͷ
	void drawDivisionLine(QPainter &painter);	//���Ʒָ��


	EditorWidgets* _pEditorWidgets = nullptr;	//ģ̬�༭��

	

	MEM_INFO _mi;	//�ڴ���Ϣ
	DataInfo _info;	//����״̬���ͱ༭���ڵ�������Ϣ

	int _middleValue = 0;	//����������ֵ

	int _nTimerID = -1;			//��ʱ��ID�������϶�������ʱ��������
	bool _bSliderMove = false;	//�Ƿ����϶���������


	
	

	std::thread* _t = nullptr;	//�߳�ָ��
	bool _bStrat_t = false;		//�߳̿��ر�־
	void _tUpdateData();		//�߳�ˢ�º���
	int _ms = 2000;				//�䶯���ݵĸ���ʱ���ʱ��
	int _interval = 330;		//��ʱ�������ļ��ʱ��
	int _nTimerID2 = -1;		//����䶯���ݵĶ�ʱ�����䶯���ݵı���ɫ�𽥱䵭
	QHash<unsigned long long, RENDER_DATA_INFO> _hashAddrTable;	//���ݱ䶯ʱ��ҪȾɫ�����ݵ�ַ��ϣ��

	
	unsigned int _dataOffset;	//���һ�е�ַ��ƫ��
	unsigned int _dataSize;		//Ҫ��ȡ�����ݴ�С


	void _paste();

	QUndoStack* _pUndoStack = nullptr;
	QStack<unsigned long long> _addrStack;
};
