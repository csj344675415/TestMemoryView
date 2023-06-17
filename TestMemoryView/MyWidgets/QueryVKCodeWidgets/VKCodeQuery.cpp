#include "VKCodeQuery.h"
#include <QEvent>
#include <QKeyEvent>

#include "Key_Information.h"

VKCodeQuery::VKCodeQuery(QWidget *parent)
	: QLineEdit(parent)
{
	setAttribute(Qt::WA_InputMethodEnabled, false); //�����������뷨
	pushKeyvector();	//�Ѱ�����Ϣ�����������
}

VKCodeQuery::~VKCodeQuery()
{
}

bool VKCodeQuery::event(QEvent *event)
{
	if (event->type() == QEvent::KeyPress) 
	{
		KEY_INFORMATION Key_Information;
		QKeyEvent *ke = static_cast<QKeyEvent *>(event); //��Ҫͷ�ļ�֧��
		std::size_t scanCode = ke->nativeVirtualKey();
		if (GetKeyInformation(Key_Information, scanCode)) {
			emit signal_keyInfo(Key_Information.Keyname, Key_Information.VK_Key, Key_Information.Key);
			return true;
		}
	}
	return QWidget::event(event);
}