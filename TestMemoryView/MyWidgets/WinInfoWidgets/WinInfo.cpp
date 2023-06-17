#include "WinInfo.h"
#include <QMouseEvent>
#include <QPainter>


WinInfo::WinInfo(QWidget *parent)
	: QLabel(parent)
{
	this->setFixedSize(32, 32);

	__pixRelease = QPixmap(":/MyWidgets/WinInfoWidgets/Resources/drag.ico");	//��������
	__pixPress = QPixmap(":/MyWidgets/WinInfoWidgets/Resources/drag2.ico");	//����
	__pixMouse = QPixmap(":/MyWidgets/WinInfoWidgets/Resources/eye.cur");		//����ʱ�Ĺ��
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
	__hWnd = GetDesktopWindow();	//��ȡ������
	__hDc = GetWindowDC(__hWnd);	//��ȡ�����ͼDC   
	SetROP2(__hDc, R2_NOTXORPEN);	//��ˢ���÷�ɫ
	__hPen = CreatePen(0, 3, RGB(255, 0, 0));      //�����߿�Ϊ3����ɫ��ʵ�߻���  
	SelectObject(__hDc, __hPen);	//������ѡ���豸 
	__okDraw = false;
	__lodHwnd = 0;

	while (!__bStop)
	{
		GetCursorPos(&__ptCursor);                //ȡ���������

		__newHwnd = WindowFromPoint(__ptCursor);    //ȡ�����ָ�봦���ھ��

		if (__newHwnd == __lodHwnd)	{
			//�����ǰ�����ȡ�������һ�������ȡ�ľ��ֵһ������ʲôҲ��������ֹ��˸
			std::this_thread::sleep_for(std::chrono::milliseconds(100));      //����
			continue;
		}
		__lodHwnd = __newHwnd;
		if (__okDraw){
			Rectangle(__hDc, __rect.left, __rect.top, __rect.right, __rect.bottom);	//�����ϸ����ڻ��Ƶı߿�
			__okDraw = !__okDraw;
		}

		GetWindowRect(__newHwnd, &__rect);      //��ô��ھ���
		//GetClientRect(NewHwnd, &rect);
		if (__rect.left < 0) __rect.left = 0;
		if (__rect.top < 0) __rect.top = 0;
		if (__rect.right > GetSystemMetrics(SM_CXSCREEN)) __rect.right = GetSystemMetrics(SM_CXSCREEN);
		//if (rect.bottom > GetSystemMetrics(SM_CYSCREEN)) rect.bottom = GetSystemMetrics(SM_CYSCREEN);

		//���ƾ��α߿�  
		Rectangle(__hDc, __rect.left, __rect.top, __rect.right, __rect.bottom);
		__okDraw = !__okDraw;
		/********************************��ȡ��Ϣ*************************************/
		emit signal_windowHwnd(__newHwnd);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));      //����
	}

	if (__okDraw){
		Rectangle(__hDc, __rect.left, __rect.top, __rect.right, __rect.bottom);	//�������Ƶı߿�
		__okDraw = !__okDraw;
	}

	DeleteObject(__hPen);		//ɾ������
	ReleaseDC(__hWnd, __hDc);	//�ͷ�DC
	__okDraw = FALSE;
}

PWINDOWS_INFOMATION WinInfo::GetWindowInfo(HWND hwnd)
{
	if (!__m_pWi) {
		__m_pWi = new _WINDOWS_INFOMATION();
	}

	__m_pWi->hwnd = hwnd;
	GetWindowText(hwnd, __m_pWi->title, 256);						//��ȡ���ڱ���
	GetClassName(hwnd, __m_pWi->className, 256);					//��ȡ��������
	__m_pWi->tid = GetWindowThreadProcessId(hwnd, &__m_pWi->pid);	//��ȡ���ڵĴ����ߣ��̻߳���̣��ı�ʶ��ID
	GetWindowRect(hwnd, &__m_pWi->rect);							//��ô��ھ���
	GetClientRect(hwnd, &__m_pWi->clientRect);						//��ÿͻ�������

	return __m_pWi;
}

void WinInfo::mousePressEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		//�������
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
		//�������
		__bStop = true;
		std::this_thread::sleep_for(std::chrono::milliseconds(100));      //����
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
	//����
	setCursor(Qt::PointingHandCursor);
}

void WinInfo::leaveEvent(QEvent* event)
{
	QWidget::leaveEvent(event);
	//����
}

//void WinInfo::paintEvent(QPaintEvent *event)
//{
//	QWidget::paintEvent(event);
//	QPainter painter(this);
//	//painter.setRenderHint(QPainter::SmoothPixmapTransform); //���������
//	drawIcon(&painter);
//}

//����ͼ��
void WinInfo::drawIcon(QPainter *painter)
{
	//// ����ԭʼ����
	//painter->save();

	//// ��ȡ��ͬ��ʾ���ķֱ��ʱ�������ֹ��ͬ�ֱ�����ͼƬʧ��
	//qreal pixelRatio = painter->device()->devicePixelRatioF();
	//// ����ͼƬ����ǰ�ֱ����µ���ʾ��С��SmoothTransformationƽ������
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

	//// �ָ�ԭʼ����
	//painter->restore();
}