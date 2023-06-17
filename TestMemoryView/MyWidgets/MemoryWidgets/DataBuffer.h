#pragma once

#include <QObject>
#include <QVector>
#include <QHash>
#include <QColor>

#include <thread>

#pragma warning(disable:4091)
#pragma warning(disable:4996)

class QTimerEvent;

#ifndef STRUCT_DataBuffer
typedef struct QMODULE_INFO_T
{
	unsigned long long moduleBaseAddr;	//模块起始地址
	unsigned long long moduleSize;		//模块大小
	QString moduleName;					//模块名
	QString modulePath;					//模块路径
	unsigned int gcount;				//全局引用计数
	unsigned int count;					//引用计数
};

typedef struct RENDER_DATA_INFO {
	int ms;				//染色剩余时长
	unsigned char diaphaneity;	//透明度
};

typedef struct MEM_INFO_T
{
	unsigned long long firstAddr;		//当前页的结束地址
	unsigned long long allocationBase;	//所有内存页对应的模块基址
	unsigned long long beginAddr;		//当前页的开始地址
	unsigned long long endAddr;			//当前页的结束地址
	unsigned int size;					//当前页的大小
	unsigned int state;					//内存状态
	unsigned int state2;				//内存状态
	unsigned int type;					//内存类型
	unsigned int protect;				//当前页的保护属性
	QString strProtect;					//页保护属性文本
	QColor color;
	QColor color2;
	QString moduleName;
};
#define STRUCT_DataBuffer
#endif

class DataBuffer : public QObject
{
	Q_OBJECT

public:
	DataBuffer(QObject *parent);
	~DataBuffer();

protected:
	void timerEvent(QTimerEvent *event);


	signals:
	void signal_dataChange();	//数据变化，发射信号
	void signal_update();		//在定时器里发射更新信号，通知View更新
public:
	MEM_INFO_T setData(unsigned int pid, unsigned long long addr = -1);
	MEM_INFO_T changeAddr(unsigned long long addr = -1);
	void changeTime(int ms);
	void updateStop();	//停止数据刷新
	void updateStart();	//开始数据刷新
	unsigned char* data() { return _buffer; }
	QHash<unsigned long long, RENDER_DATA_INFO>* getHashTable() { return &_hashAddrTable; }

protected:
	void getModuleInfo(unsigned int pid);

	unsigned int _pid;
	std::thread* _t = nullptr;	//线程指针
	bool _bStrat_t = false;	//线程开关标志
	void _tUpdateData();		//线程刷新函数
	int _nTimerID2 = -1;		//处理变动数据的定时器，变动数据的背景色逐渐变淡
	int _interval = 330;		//定时器出发的间隔时间
	unsigned int _dataOffset = 0;
	unsigned int _dataSize = 0;
	QVector<QMODULE_INFO_T> _vecModule;

	unsigned char _buffer[1024];
	unsigned long long _firstAddr = -1;
	int _maxBytes = 1024;
	
	MEM_INFO_T _strMI;

	int _ms = 1000;				//变动数据的高亮时间最长时间
	QHash<unsigned long long, RENDER_DATA_INFO> _hashAddrTable;	//数据变动时需要染色的数据地址哈希表
};
