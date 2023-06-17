#pragma once
#include <QMetaType>	//ItemStatus
#include <QString>	
#include <QColor>	
#include <QDateTime>
#include <QIcon>
#include <QVariant>
#include <QFont>

enum {
	NULL_COLUMN = -1,
	CHECK_BOX_COLUMN = 0,
	ONE_COLUMN,
	TOW_COLUMN,
	THREE_COLUMN,
	FOUR_COLUMN,
	FIVE_COLUMN,
	SIX_COLUMN,
	SEVEN_COLUMN,
	EIGHT_COLUMN,
	NINE_COLUMN,
	TEN_COLUMN,
};

typedef struct _HEADER_DATA {
	QString headerData;			//表头文本
	QString icon;				//图标路径
	QColor color;				//字体颜色
	Qt::Alignment alignment;	//字体对齐方式
} HEADER_DATA, *PHEADER_DATA;
Q_DECLARE_METATYPE(_HEADER_DATA)

typedef struct _ITEM_DATA {
	bool bChecked;				//是否选中
	QVariant data;				//显示在界面上数据
	QVariant userData;			//用户数据，不直接显示，用于其他操作，如自绘等
	QIcon icon;					//图标路径
	QColor color;				//字体颜色
	Qt::Alignment alignment;	//字体对齐方式
	QVariant strToolTip;		//提示标签
	QFont font;					//字体
	QColor colorBackground;		//背景色
	QColor colorForeground;		//前景色
} ITEM_DATA, *PITEM_DATA;
Q_DECLARE_METATYPE(_ITEM_DATA)

//对于自定义数据类型，如果要使用QVariant，就必须使用Q_DECLARE_METATYPE注册。

//QVariant(可变体)，内置了很多数据类型