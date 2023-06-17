#include "WinInfo.h"
#include <QMouseEvent>
#include <QPainter>


WinInfo::WinInfo(QWidget *parent)
	: QLabel(parent)
{
	this->setFixedSize(32, 32);

	__pixRelease = QPixmap(":/MyWidgets/WinInfoWidgets/Resources/drag.ico");	//正常或弹起
	__pixPress = QPixmap(":/MyWidgets/WinInfoWidgets/Resources/drag2.ico");	//按下
	__pixMouse = QPixmap(":/MyWidgets/WinInfoWidgets/Resources/eye.cur");		//按下时的光标
	this->setPixmap(__pixRelease);
}

WinInfo::~WinInfo()
{
	if (__thread_1.joinable()) {
		__thread_1.join();
	}
	if(__m_pWi) delete __m_pWi;
}

void WinInfo::__thread_getWindowsInfo()
{
	__hWnd = GetDesktopWindow();	//获取桌面句柄
	__hDc = GetWindowDC(__hWnd);	//获取桌面绘图DC   
	SetROP2(__hDc, R2_NOTXORPEN);	//画刷设置反色
	__hPen = CreatePen(0, 3, RGB(255, 0, 0));      //创建线宽为3，黑色的实线画笔  
	SelectObject(__hDc, __hPen);	//将画笔选入设备 
	__okDraw = false;
	__lodHwnd = 0;

	while (!__bStop)
	{
		GetCursorPos(&__ptCursor);                //取得鼠标坐标

		__newHwnd = WindowFromPoint(__ptCursor);    //取得鼠标指针处窗口句柄

		if (__newHwnd == __lodHwnd)	{
			//如果当前鼠标点获取句柄与上一个鼠标点获取的句柄值一样，则什么也不做，防止闪烁
			std::this_thread::sleep_for(std::chrono::milliseconds(100));      //毫秒
			continue;
		}
		__lodHwnd = __newHwnd;
		if (__okDraw){
			Rectangle(__hDc, __rect.left, __rect.top, __rect.right, __rect.bottom);	//擦除上个窗口绘制的边框
			__okDraw = !__okDraw;
		}

		GetWindowRect(__newHwnd, &__rect);      //获得窗口矩形
		//GetClientRect(NewHwnd, &rect);
		if (__rect.left < 0) __rect.left = 0;
		if (__rect.top < 0) __rect.top = 0;
		if (__rect.right > GetSystemMetrics(SM_CXSCREEN)) __rect.right = GetSystemMetrics(SM_CXSCREEN);
		//if (rect.bottom > GetSystemMetrics(SM_CYSCREEN)) rect.bottom = GetSystemMetrics(SM_CYSCREEN);

		//绘制矩形边框  
		Rectangle(__hDc, __rect.left, __rect.top, __rect.right, __rect.bottom);
		__okDraw = !__okDraw;
		/********************************获取信息*************************************/
		emit signal_windowHwnd(__newHwnd);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));      //毫秒
	}

	if (__okDraw){
		Rectangle(__hDc, __rect.left, __rect.top, __rect.right, __rect.bottom);	//擦除绘制的边框
		__okDraw = !__okDraw;
	}

	DeleteObject(__hPen);		//删除画笔
	ReleaseDC(__hWnd, __hDc);	//释放DC
	__okDraw = FALSE;
}

PWINDOWS_INFOMATION WinInfo::GetWindowInfo(HWND hwnd)
{
	if (!__m_pWi) {
		__m_pWi = new _WINDOWS_INFOMATION();
	}

	__m_pWi->hwnd = hwnd;
	GetWindowText(hwnd, __m_pWi->title, 256);						//获取窗口标题
	GetClassName(hwnd, __m_pWi->className, 256);					//获取窗口类名
	__m_pWi->tid = GetWindowThreadProcessId(hwnd, &__m_pWi->pid);	//获取窗口的创建者（线程或进程）的标识符ID
	GetWindowRect(hwnd, &__m_pWi->rect);							//获得窗口矩形
	GetClientRect(hwnd, &__m_pWi->clientRect);						//获得客户区矩形

	return __m_pWi;
}

void WinInfo::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		//按下左键
		QCursor cursor;
		cursor = QCursor(__pixMouse, -1, -1);
		setCursor(cursor);

		this->setPixmap(__pixPress);

		if (__thread_1.joinable()) {
			__thread_1.join();
		}

		if (!__thread_1.native_handle()) {
			__bStop = false;
			__thread_1 = std::thread(&WinInfo::__thread_getWindowsInfo, this);
		}
	}
}

void WinInfo::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		//弹起左键
		__bStop = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));      //毫秒
		setCursor(Qt::ArrowCursor);
		emit signal_mouseRelease();
		Sleep(50);
		this->setPixmap(__pixRelease);
		//SetForegroundWindow((HWND)this->winId());
		//repaint();	
	}
}

void WinInfo::enterEvent(QEvent *event)
{
	QWidget::enterEvent(event);
	//代码
	setCursor(Qt::PointingHandCursor);
}

void WinInfo::leaveEvent(QEvent* event)
{
	QWidget::leaveEvent(event);
	//代码
}

//void WinInfo::paintEvent(QPaintEvent *event)
//{
//	QWidget::paintEvent(event);
//	QPainter painter(this);
//	//painter.setRenderHint(QPainter::SmoothPixmapTransform); //开启抗锯齿
//	drawIcon(&painter);
//}

//绘制图标
void WinInfo::drawIcon(QPainter *painter)
{
	//// 保存原始画板
	//painter->save();

	//// 获取不同显示器的分辨率比例，防止不同分辨率下图片失真
	//qreal pixelRatio = painter->device()->devicePixelRatioF();
	//// 缩放图片到当前分辨率下的显示大小，SmoothTransformation平滑处理。
	//QPixmap pixmap;

	//if (m_isEnter) {
	//	pixmap = m_icon.scaled(QSize((width() + m_offsetW - 3) * pixelRatio, (height() + m_offsetH - 3) *pixelRatio), Qt::IgnoreAspectRatio);
	//	if (m_isPress) {
	//		painter->drawPixmap(2 + m_offsetX, 2 + m_offsetY, pixmap);
	//	}
	//	else {
	//		painter->drawPixmap(1 + m_offsetX, 1 + m_offsetY, pixmap);
	//	}
	//}
	//else {
	//	pixmap = m_icon.scaled(QSize((width() + m_offsetW - 1) * pixelRatio, (height() + m_offsetH - 1) *pixelRatio), Qt::IgnoreAspectRatio);
	//	painter->drawPixmap(0 + m_offsetX, 0 + m_offsetY, pixmap);
	//}

	//// 恢复原始画板
	//painter->restore();
}