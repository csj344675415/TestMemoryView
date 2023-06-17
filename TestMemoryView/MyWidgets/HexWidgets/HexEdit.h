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
	unsigned long long allocationBase;	//所有内存页对应的模块基址
	unsigned long long beginAddr;		//当前页的开始地址
	unsigned long long endAddr;			//当前页的结束地址
	unsigned int size;					//当前页的大小
	unsigned int state;					//内存状态
	unsigned int state2;					//内存状态
	unsigned int type;					//内存类型
	unsigned int protect;				//当前页的保护属性
	QString strProtect;					//页保护属性文本
	QColor color;
	QColor color2;
	bool isBase;						//是基址范围
	QString moduleName;
};

typedef struct HexEditItem
{
	QRect rect;
	QString text;
};

typedef struct RENDER_DATA_INFO {
	int ms;				//染色剩余时长
	unsigned char diaphaneity;	//透明度
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
	void timerEvent(QTimerEvent *event); //声明时间
	//void keyPressEvent(QKeyEvent * event);
	void mouseMoveEvent(QMouseEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseReleaseEvent(QMouseEvent * event);
	void mouseDoubleClickEvent(QMouseEvent *event); //鼠标双击事件
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
	void updateStop();	//停止数据刷新
	void updateStart();	//开始数据刷新
	void changeAddr(unsigned long long addr);
	void updeteView();

protected:
	void init();
	void initMenu();
	QMenu* _pMenu = nullptr;
	QVector<QAction*> _vecAction_DataType;		//数据类型
	QVector<QAction*> _vecAction_TextType;		//文本编码
	QVector<QAction*> _vecAction_divisionType;	//分割符
	QVector<QAction*> _vecAction_ProtectType;	//页面保护属性

	void _updateViewItem();
	QFontMetrics* _pFontMetrics;
	int _rowHeight = 0;			//一行的高度
	int _maxRows = 0;			//视图可显示的最大行数
	int _maxAddrWidth = 0;		//地址字符串的宽度
	int _maxDataWidth = 0;		//数据字符串的宽度
	int _maxTextWidth = 0;		//文本字符串的宽度
	int _dataCols = 0;			//视图可显示数据的列数
	int _textCols = 0;			//视图可显示文本数据的列数
	int _rtAddrItemWidth = 0;	//地址Item的宽度
	int _rtDataItemWidth = 0;	//数据Item的宽度
	int _rtTextItemWidth = 0;	//文本Item的宽度
	unsigned int _maxBytes = 1024;	//视图可显示的最大字节数


	int _getStringWidth(int id);	//获取字符串宽度
	int getDataItemWidth();			//获取数据区域一个Item的宽度


	QRect _rtMemInfoArea;	//内存信息区域
	QRect _rtOffsetArea;	//地址偏移区域
	QRect _rtAddrArea;		//地址区域
	QRect _rtDataArea;		//数据区域
	QRect _rtTextArea;		//文本区域
	QRect _rtClickedArea;	//点击的Item区域

	HexEditItem _ProtectItem;				//保护属性
	HexEditItem _AllocationBaseAddrItem;	//当前内存块所属的模块基址
	HexEditItem _BaseAddrItem;				//内存块基址
	HexEditItem _AddrSizeItem;				//内存块大小
	HexEditItem _ModuleNameItem;			//模块名
	HexEditItem _AddrHeadItem;				//
	QVector<HexEditItem> _vecAddrItem;
	QVector<HexEditItem> _vecDataItem;
	QVector<HexEditItem> _vecTextItem;


	unsigned int _pid = 0;			//进程PID
	unsigned char _pData[1024];		//数据缓冲区
	




	unsigned long long _firstAddr = -1;	//第一个数据的实际地址
	int _divisionByte = 4;				//初始4字节分割


	bool _bPress = false;	//鼠标是否按下

	inline void setBeginPos(QPoint &pos);
	inline void setEndPos(QPoint &pos);

	const HexEditItem *_pLastMouseItem = nullptr;	//上一次鼠标指向的Item，当前鼠标指向的Item与_lastMouseItem不相同时，才会进行刷新
	bool _bClicked = false;

	unsigned long long _dataAddr1 = 0;		//鼠标按下时，指向的Item里存放的数据对应的地址
	unsigned long long _dataAddr2 = 0;		//鼠标移动或弹起时，指向的Item里存放的数据对应的地址
	unsigned long long _headAddr1 = 0;		//鼠标按下时，指向的Item所在行的头地址
	unsigned long long _headAddr2 = 0;		//鼠标移动或弹起时，指向的Item所在行的头地址
	unsigned int _dataIndex1 = 0;			//鼠标按下时，指向的Item里存放的数据位于缓冲区里的索引
	unsigned int _dataIndex2 = 0;			//鼠标移动或弹起时，指向的Item里存放的数据位于缓冲区里的索引
	unsigned int _itemIndex1 = 0;
	unsigned int _itemIndex2 = 0;
	unsigned int _colIndex = 0;				//鼠标点击时，指向的Item对应的列索引
	//bool _bSwap = false;					//是否交换地址，当 _dataAddr2 小于 _dataAddr1 时需要交换
	int _releaseArea = 0;					//鼠标弹起时位于哪个区域，数据区域 = 1，位于文本区域 = 2，其他区域 = 0


	int _dataType = e1ByteHex;	//当前显示的数据类型
	int _textType = eASCII;		//当前显示的文本编码
	bool _bSymbol = false;		//是否显示符号（10进制时是否显示 ‘-’ 号）

	QString getHexData(unsigned long long addr, long long index);	//取得对应数据类型的字符串
	unsigned int getTextData(unsigned long long addr, long long index, int &itemIndex, QString &text);	//取得对应数据类型的字符串


	QColor _penColor;		//画笔颜色
	QFont _originalFont;	//原始字体
	QFont _boldFont;		//加粗字体
	void drawMemoryInfo(QPainter &painter);					//绘制内存信息
	void drawAddress(QPainter &painter);	//绘制地址
	void drawData(QPainter &painter);		//绘制数据
	void drawTextData(QPainter &painter);	//绘制文本数据
	void drawDataHead(QPainter &painter);		//绘制数据区域表头
	void drawTextDataHead(QPainter &painter);	//绘制文本区域表头
	void drawDivisionLine(QPainter &painter);	//绘制分割符


	EditorWidgets* _pEditorWidgets = nullptr;	//模态编辑框

	

	MEM_INFO _mi;	//内存信息
	DataInfo _info;	//传给状态栏和编辑窗口的数据信息

	int _middleValue = 0;	//滚动条居中值

	int _nTimerID = -1;			//定时器ID，用于拖动滚动条时滚动数据
	bool _bSliderMove = false;	//是否在拖动滚动条条


	
	

	std::thread* _t = nullptr;	//线程指针
	bool _bStrat_t = false;		//线程开关标志
	void _tUpdateData();		//线程刷新函数
	int _ms = 2000;				//变动数据的高亮时间最长时间
	int _interval = 330;		//定时器出发的间隔时间
	int _nTimerID2 = -1;		//处理变动数据的定时器，变动数据的背景色逐渐变淡
	QHash<unsigned long long, RENDER_DATA_INFO> _hashAddrTable;	//数据变动时需要染色的数据地址哈希表

	
	unsigned int _dataOffset;	//与第一行地址的偏移
	unsigned int _dataSize;		//要读取的数据大小


	void _paste();

	QUndoStack* _pUndoStack = nullptr;
	QStack<unsigned long long> _addrStack;
};
