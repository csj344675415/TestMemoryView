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

	void timerEvent(QTimerEvent *event); //声明时间
	void resizeEvent(QResizeEvent * event);
	void paintEvent(QPaintEvent * event);
	void wheelEvent(QWheelEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mouseDoubleClickEvent(QMouseEvent *event); //鼠标双击事件
	void closeEvent(QCloseEvent *event);

signals:
	void signal_clickedItem(DataInfo info);
	void signal_resize(int w);

	public slots:
	// 滚动条
	void slot_sliderPressed();
	void slot_sliderReleased();
	void slot_actionTriggered(int value);

	// 菜单
	void slot_editAddr();
	void slot_dataType();
	void slot_textType();
	void slot_divisionType();
	void slot_symbol();
	void slot_protectType();
	void slot_intoAddr();
	void slot_toAddr();
	void slot_setTime();

	// 快捷键
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

	// 滚动条相关
	int _middleValue = 0;
	bool _bSliderMove = false;	//是否在拖动滑块
	int _nTimerID = -1;
	unsigned long long _firstAddr = -1;	//第一个数据的实际地址

	QMenu* _pMenu = nullptr;
	QVector<QAction*> _vecAction_DataType;
	QVector<QAction*> _vecAction_TextType;
	QVector<QAction*> _vecAction_divisionType;
	

	// Item相关
	void setItem();
	QFontMetrics *_pMetrics = nullptr;	//字体计算类
	int _rowHeight = 0;			//一行的高度
	int _maxRowCount = 0;		//最大行数
	int _maxByteCount = 1024;	//最大字节数
	int _addrStringWidth = 0;	//一个地址字符串的最大宽度
	int _dataStringWidth = 0;	//一个数据字符串的最大宽度
	int _textStringWidth = 0;	//一个文本字符串的最大宽度
	int _addrItemWidth = 0;		//一个地址Item的宽度
	int _dataItemWidth = 0;		//一个数据Item的宽度
	int _textItemWidth = 0;		//一个文本Item的宽度
	int _dataColumnCount = 0;
	int _textColumnCount = 0;
	int _maxWidgetWidth = 0;	//窗口适应数据的宽度
	int getStringWidth(int id);
	int getDataItemWidth();

	int _dataType = e1ByteHex;
	int _textType = eUTF8;
	bool _bSymbol = false;		//10进制时是否带符号显示

	QRect _rtMemInfoArea;	//内存信息区域
	QRect _rtAddrHeadItem;	
	QRect _rtOffsetArea;	//地址偏移区域
	QRect _rtAddrArea;		//地址区域
	QRect _rtDataArea;		//数据区域
	QRect _rtTextArea;		//文本区域
	QRect _rtClickedArea;	//点击的Item区域

	HexEditItem _strProtectItem;				//保护属性
	HexEditItem _strAllocationBaseAddrItem;		//当前内存块所属的模块基址
	HexEditItem _strBaseAddrItem;				//内存块基址
	HexEditItem _strAddrSizeItem;				//内存块大小
	HexEditItem _strModuleNameItem;				//模块名
	HexEditItem _strAddrHeadItem;				//
	QVector<HexEditItem> _vecAddrItem;
	QVector<HexEditItem> _vecDataItem;
	QVector<HexEditItem> _vecTextItem;


	

	// 绘制相关
	void drawMemoryInfo(QPainter &painter);		//绘制内存信息
	void drawAddress(QPainter &painter);		//绘制地址
	void drawData(QPainter &painter);			//绘制数据
	void drawTextData(QPainter &painter);		//绘制文本数据
	void drawDataHead(QPainter &painter);		//绘制数据区域表头
	void drawTextDataHead(QPainter &painter);	//绘制文本区域表头
	void drawDivisionLine(QPainter &painter);	//绘制分割符

	QString getHexData(unsigned long long addr, long long index);	//取得对应数据类型的字符串
	unsigned int getTextData(unsigned long long addr, long long index, int &itemIndex, QString &text);	//取得对应数据类型的字符串
	unsigned char u8bytes(unsigned char byte);	//获取一个UTF-8字符所占字节数
	inline int preNum(unsigned char byte);		//获取一个UTF-8字符所占字节数
	bool isUtf8Code(const char* data);			//判断是否为UTF8编码

	QColor _penColor;
	int _divisionByte = 4;
	MEM_INFO_T _mi;


	// 点击数据相关
	void setBeginPos(QPoint &pos);
	void setEndPos(QPoint &pos);
	bool _bPress = false;		//鼠标按下
	bool _bClicked = false;		//点击Item

	unsigned long long _dataAddr1 = 0;		//鼠标按下时，指向的Item里存放的数据对应的地址
	unsigned long long _dataAddr2 = 0;		//鼠标移动或弹起时，指向的Item里存放的数据对应的地址
	unsigned long long _headAddr1 = 0;		//鼠标按下时，指向的Item所在行的头地址
	unsigned long long _headAddr2 = 0;		//鼠标移动或弹起时，指向的Item所在行的头地址
	unsigned int _dataIndex1 = 0;			//鼠标按下时，指向的Item里存放的数据位于缓冲区里的索引
	unsigned int _dataIndex2 = 0;			//鼠标移动或弹起时，指向的Item里存放的数据位于缓冲区里的索引
	unsigned int _itemIndex1 = 0;
	unsigned int _itemIndex2 = 0;
	unsigned int _colIndex = 0;				//鼠标点击时，指向的Item对应的列索引
	int _releaseArea = 0;					//鼠标弹起时位于哪个区域，数据区域 = 1，位于文本区域 = 2 ，复制时用到
	unsigned long long _dataAddr1_backup = 0;	//_dataAddr1 的备份
	unsigned long long _headAddr1_backup = 0;	//_headAddr1 的备份
	unsigned long long _dataIndex1_backup = 0;	//_dataIndex1 的备份
	unsigned long long _itemIndex1_backup = 0;	//_dataIndex1 的备份
	const HexEditItem *_pLastItem = nullptr;	//上一次鼠标指向的Item，当前鼠标指向的Item与_pLastItem不相同时，才会进行刷新

	unsigned long long _relativeAddress = 0;	//相对地址
	bool _bDrawRelativeAddress = false;

	DataInfo _dataInfo;
	QStack<unsigned long long> _addrStack;


	
	QStack<Into_Stack_Data> _intoStack1;
	//QStack<Into_Stack_Data> _intoStack2;

};

