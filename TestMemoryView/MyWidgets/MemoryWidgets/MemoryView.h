#pragma once

#include <QAbstractScrollArea>
#include <QStack>
//#include <QFontMetrics>

#include "DataBuffer.h"
#include "EditorWidgets.h"
#include "MemoryViewCommand.h"

#pragma warning(disable:4091)
#pragma warning(disable:4996)

class QMenu;
class QAction;
class QUndoStack;



#ifndef STRUCT_MemoryView
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

typedef struct HexEditItem
{
	QRect rect;
	QString text;
};

typedef struct Into_Stack_Data
{
	unsigned long long headAddr;
	unsigned long long intoAddr;
	unsigned long long retAddr;
	unsigned int intoSelect;
	unsigned int retSelect;
	unsigned int dataIndex;
	unsigned int colIndex;

};
#define STRUCT_MemoryView
#endif



class MemoryView : public QAbstractScrollArea
{
	Q_OBJECT

public:
	MemoryView(QWidget *parent = Q_NULLPTR);
	~MemoryView();

private:

	void timerEvent(QTimerEvent *event); //����ʱ��
	void resizeEvent(QResizeEvent * event);
	void paintEvent(QPaintEvent * event);
	void wheelEvent(QWheelEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mouseDoubleClickEvent(QMouseEvent *event); //���˫���¼�
	void closeEvent(QCloseEvent *event);

signals:
	void signal_clickedItem(DataInfo info);
	void signal_resize(int w);

	public slots:
	// ������
	void slot_sliderPressed();
	void slot_sliderReleased();
	void slot_actionTriggered(int value);

	// �˵�
	void slot_editAddr();
	void slot_dataType();
	void slot_textType();
	void slot_divisionType();
	void slot_symbol();
	void slot_protectType();
	void slot_intoAddr();
	void slot_toAddr();
	void slot_setTime();

	// ��ݼ�
	void slot_copy();
	void slot_paste();
	void slot_undo();
	void slot_redo();
	void slot_backspaceAddr();
	void slot_backspaceIntoAddr1();
	void slot_backspaceIntoAddr2();

	//
	void slot_changeData(DataInfo di);
	//void slot_changeAddr(unsigned long long addr);
	void slot_changeAddr2(unsigned long long addr);
	void slot_changeTime(int ms);
	
	
	
	
public:
	void setData(unsigned int pid, unsigned long long addr = -1);
	void updateStop();
	void show(unsigned int pid, unsigned long long addr = -1);
protected:
	DataBuffer* _buffer = nullptr;
	EditorWidgets* _pEditor = nullptr;
	QUndoStack* _pUndoStack = nullptr;

	void init();
	void initMenu();
	void initShortcut();

	unsigned int _pid = 0;

	// ���������
	int _middleValue = 0;
	bool _bSliderMove = false;	//�Ƿ����϶�����
	int _nTimerID = -1;
	unsigned long long _firstAddr = -1;	//��һ�����ݵ�ʵ�ʵ�ַ

	QMenu* _pMenu = nullptr;
	QVector<QAction*> _vecAction_DataType;
	QVector<QAction*> _vecAction_TextType;
	QVector<QAction*> _vecAction_divisionType;
	

	// Item���
	void setItem();
	QFontMetrics *_pMetrics = nullptr;	//���������
	int _rowHeight = 0;			//һ�еĸ߶�
	int _maxRowCount = 0;		//�������
	int _maxByteCount = 1024;	//����ֽ���
	int _addrStringWidth = 0;	//һ����ַ�ַ����������
	int _dataStringWidth = 0;	//һ�������ַ����������
	int _textStringWidth = 0;	//һ���ı��ַ����������
	int _addrItemWidth = 0;		//һ����ַItem�Ŀ��
	int _dataItemWidth = 0;		//һ������Item�Ŀ��
	int _textItemWidth = 0;		//һ���ı�Item�Ŀ��
	int _dataColumnCount = 0;
	int _textColumnCount = 0;
	int _maxWidgetWidth = 0;	//������Ӧ���ݵĿ��
	int getStringWidth(int id);
	int getDataItemWidth();

	int _dataType = e1ByteHex;
	int _textType = eUTF8;
	bool _bSymbol = false;		//10����ʱ�Ƿ��������ʾ

	QRect _rtMemInfoArea;	//�ڴ���Ϣ����
	QRect _rtAddrHeadItem;	
	QRect _rtOffsetArea;	//��ַƫ������
	QRect _rtAddrArea;		//��ַ����
	QRect _rtDataArea;		//��������
	QRect _rtTextArea;		//�ı�����
	QRect _rtClickedArea;	//�����Item����

	HexEditItem _strProtectItem;				//��������
	HexEditItem _strAllocationBaseAddrItem;		//��ǰ�ڴ��������ģ���ַ
	HexEditItem _strBaseAddrItem;				//�ڴ���ַ
	HexEditItem _strAddrSizeItem;				//�ڴ���С
	HexEditItem _strModuleNameItem;				//ģ����
	HexEditItem _strAddrHeadItem;				//
	QVector<HexEditItem> _vecAddrItem;
	QVector<HexEditItem> _vecDataItem;
	QVector<HexEditItem> _vecTextItem;


	

	// �������
	void drawMemoryInfo(QPainter &painter);		//�����ڴ���Ϣ
	void drawAddress(QPainter &painter);		//���Ƶ�ַ
	void drawData(QPainter &painter);			//��������
	void drawTextData(QPainter &painter);		//�����ı�����
	void drawDataHead(QPainter &painter);		//�������������ͷ
	void drawTextDataHead(QPainter &painter);	//�����ı������ͷ
	void drawDivisionLine(QPainter &painter);	//���Ʒָ��

	QString getHexData(unsigned long long addr, long long index);	//ȡ�ö�Ӧ�������͵��ַ���
	unsigned int getTextData(unsigned long long addr, long long index, int &itemIndex, QString &text);	//ȡ�ö�Ӧ�������͵��ַ���
	unsigned char u8bytes(unsigned char byte);	//��ȡһ��UTF-8�ַ���ռ�ֽ���
	inline int preNum(unsigned char byte);		//��ȡһ��UTF-8�ַ���ռ�ֽ���
	bool isUtf8Code(const char* data);			//�ж��Ƿ�ΪUTF8����

	QColor _penColor;
	int _divisionByte = 4;
	MEM_INFO_T _mi;


	// ����������
	void setBeginPos(QPoint &pos);
	void setEndPos(QPoint &pos);
	bool _bPress = false;		//��갴��
	bool _bClicked = false;		//���Item

	unsigned long long _dataAddr1 = 0;		//��갴��ʱ��ָ���Item���ŵ����ݶ�Ӧ�ĵ�ַ
	unsigned long long _dataAddr2 = 0;		//����ƶ�����ʱ��ָ���Item���ŵ����ݶ�Ӧ�ĵ�ַ
	unsigned long long _headAddr1 = 0;		//��갴��ʱ��ָ���Item�����е�ͷ��ַ
	unsigned long long _headAddr2 = 0;		//����ƶ�����ʱ��ָ���Item�����е�ͷ��ַ
	unsigned int _dataIndex1 = 0;			//��갴��ʱ��ָ���Item���ŵ�����λ�ڻ������������
	unsigned int _dataIndex2 = 0;			//����ƶ�����ʱ��ָ���Item���ŵ�����λ�ڻ������������
	unsigned int _itemIndex1 = 0;
	unsigned int _itemIndex2 = 0;
	unsigned int _colIndex = 0;				//�����ʱ��ָ���Item��Ӧ��������
	int _releaseArea = 0;					//��굯��ʱλ���ĸ������������� = 1��λ���ı����� = 2 ������ʱ�õ�
	unsigned long long _dataAddr1_backup = 0;	//_dataAddr1 �ı���
	unsigned long long _headAddr1_backup = 0;	//_headAddr1 �ı���
	unsigned long long _dataIndex1_backup = 0;	//_dataIndex1 �ı���
	unsigned long long _itemIndex1_backup = 0;	//_dataIndex1 �ı���
	const HexEditItem *_pLastItem = nullptr;	//��һ�����ָ���Item����ǰ���ָ���Item��_pLastItem����ͬʱ���Ż����ˢ��

	unsigned long long _relativeAddress = 0;	//��Ե�ַ
	bool _bDrawRelativeAddress = false;

	DataInfo _dataInfo;
	QStack<unsigned long long> _addrStack;


	
	QStack<Into_Stack_Data> _intoStack1;
	//QStack<Into_Stack_Data> _intoStack2;

};

