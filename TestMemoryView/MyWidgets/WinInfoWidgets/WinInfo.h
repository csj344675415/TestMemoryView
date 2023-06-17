#pragma once

#include <QLabel> 
// ϵͳ��
#define WIN32_LEAN_AND_MEAN
#include <windows.h> 
// ��׼��
#include <thread>
// ��������

// �Զ����

//���������Ϣ�ṹ��
typedef struct _WINDOWS_INFOMATION
{
	HWND hwnd;				//���ھ��
	wchar_t title[256];		//���ڱ���
	wchar_t className[64];	//��������
	DWORD pid;				//����PID
	DWORD tid;				//���߳�TID
	RECT rect;				//���ڴ�С
	RECT clientRect;		//�ͻ�����С
}WINDOWS_INFOMATION, *PWINDOWS_INFOMATION;

class WinInfo : public QLabel
{
	Q_OBJECT

public:
	WinInfo(QWidget *parent = Q_NULLPTR);
	~WinInfo();

	protected slots:  //��

signals:          //�ź�
	void signal_windowHwnd(HWND hwnd);
	void signal_mouseRelease();	//��굯��

private: //˽��
	//QLabel *__pLabel = nullptr;
	QPixmap __pixPress;		//����ʱ��ͼ��
	QPixmap __pixRelease;	//��������ʱ��ͼ��
	QPixmap __pixMouse;		//����ʱ�������ʽ

	void __thread_getWindowsInfo();
	std::thread __thread_1;
	bool __bStop = false;

	//����׼��ͼ�����
	HWND __hWnd;	//������
	HDC __hDc;		//DC�豸��
	HPEN __hPen;;	//������
	BOOL __okDraw;	//���Ʊ�ʶ

	//���Ʊ߿����
	POINT __ptCursor;
	RECT __rect;		//Ҫ���Ƶľ��δ�С
	HWND __lodHwnd;		//��һ�����ھ��
	HWND __newHwnd;		//�´��ھ��

	PWINDOWS_INFOMATION __m_pWi = nullptr;
protected: //�ܱ���
	void mousePressEvent(QMouseEvent *event);	//��갴���¼�
	void mouseReleaseEvent(QMouseEvent *event); //��굯���¼�
	void enterEvent(QEvent *event);				//�����봰���¼�
	void leaveEvent(QEvent* event);				//����뿪�����¼�
	//void paintEvent(QPaintEvent *event);		//�����¼�

public:  //����
	/**********************************����***************************************/

	/**********************************���غ���***********************************/

	/**********************************��Ϣ��Ӧ����*******************************/

	/**********************************������*************************************/

	/**********************************�ؼ���Ӧ����*******************************/

	/**********************************�ؼ�Control��******************************/

	/**********************************�ؼ���������*******************************/

	/**********************************�Զ��庯��*********************************/
	PWINDOWS_INFOMATION GetWindowInfo(HWND hwnd);	//��ȡ������Ϣ
	/**********************************�Զ������*********************************/
	
	/**********************************������*************************************/



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
