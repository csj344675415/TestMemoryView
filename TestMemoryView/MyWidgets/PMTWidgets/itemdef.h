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
	QString headerData;			//��ͷ�ı�
	QString icon;				//ͼ��·��
	QColor color;				//������ɫ
	Qt::Alignment alignment;	//������뷽ʽ
} HEADER_DATA, *PHEADER_DATA;
Q_DECLARE_METATYPE(_HEADER_DATA)

typedef struct _ITEM_DATA {
	bool bChecked;				//�Ƿ�ѡ��
	QVariant data;				//��ʾ�ڽ���������
	QVariant userData;			//�û����ݣ���ֱ����ʾ�������������������Ի��
	QIcon icon;					//ͼ��·��
	QColor color;				//������ɫ
	Qt::Alignment alignment;	//������뷽ʽ
	QVariant strToolTip;		//��ʾ��ǩ
	QFont font;					//����
	QColor colorBackground;		//����ɫ
	QColor colorForeground;		//ǰ��ɫ
} ITEM_DATA, *PITEM_DATA;
Q_DECLARE_METATYPE(_ITEM_DATA)

//�����Զ����������ͣ����Ҫʹ��QVariant���ͱ���ʹ��Q_DECLARE_METATYPEע�ᡣ

//QVariant(�ɱ���)�������˺ܶ���������