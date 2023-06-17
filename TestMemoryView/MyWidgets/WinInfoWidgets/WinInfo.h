#pragma once

#include <QLabel> 
// 系统库
#define WIN32_LEAN_AND_MEAN
#include <windows.h> 
// 标准库
#include <thread>
// 第三方库

// 自定义库

//窗口相关信息结构体
typedef struct _WINDOWS_INFOMATION
{
	HWND hwnd;				//窗口句柄
	wchar_t title[256];		//窗口标题
	wchar_t className[64];	//窗口类名
	DWORD pid;				//进程PID
	DWORD tid;				//主线程TID
	RECT rect;				//窗口大小
	RECT clientRect;		//客户区大小
}WINDOWS_INFOMATION, *PWINDOWS_INFOMATION;

class WinInfo : public QLabel
{
	Q_OBJECT

public:
	WinInfo(QWidget *parent = Q_NULLPTR);
	~WinInfo();

	protected slots:  //槽

signals:          //信号
	void signal_windowHwnd(HWND hwnd);
	void signal_mouseRelease();	//鼠标弹起

private: //私有
	//QLabel *__pLabel = nullptr;
	QPixmap __pixPress;		//按下时的图标
	QPixmap __pixRelease;	//正常或弹起时的图标
	QPixmap __pixMouse;		//按下时的鼠标样式

	void __thread_getWindowsInfo();
	std::thread __thread_1;
	bool __bStop = false;

	//按下准心图标相关
	HWND __hWnd;	//桌面句柄
	HDC __hDc;		//DC设备类
	HPEN __hPen;;	//画笔类
	BOOL __okDraw;	//绘制标识

	//绘制边框相关
	POINT __ptCursor;
	RECT __rect;		//要绘制的矩形大小
	HWND __lodHwnd;		//上一个窗口句柄
	HWND __newHwnd;		//新窗口句柄

	PWINDOWS_INFOMATION __m_pWi = nullptr;
protected: //受保护
	void mousePressEvent(QMouseEvent *event);	//鼠标按下事件
	void mouseReleaseEvent(QMouseEvent *event); //鼠标弹起事件
	void enterEvent(QEvent *event);				//鼠标进入窗口事件
	void leaveEvent(QEvent* event);				//鼠标离开窗口事件
	//void paintEvent(QPaintEvent *event);		//绘制事件

public:  //公有
	/**********************************其他***************************************/

	/**********************************重载函数***********************************/

	/**********************************消息响应函数*******************************/

	/**********************************窗口类*************************************/

	/**********************************控件响应函数*******************************/

	/**********************************控件Control类******************************/

	/**********************************控件关联变量*******************************/

	/**********************************自定义函数*********************************/
	PWINDOWS_INFOMATION GetWindowInfo(HWND hwnd);	//获取窗口信息
	/**********************************自定义变量*********************************/
	
	/**********************************待分类*************************************/



private:
	bool m_isEnter = false;
	bool m_isPress = false;
	QPixmap m_icon;
	int m_offsetX = 0;
	int m_offsetY = 0;
	int m_offsetW = 0;
	int m_offsetH = 0;
	QRect m_rect;
	void drawIcon(QPainter *painter);
};
