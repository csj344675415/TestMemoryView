#include "VKCodeQuery.h"
#include <QEvent>
#include <QKeyEvent>

#include "Key_Information.h"

VKCodeQuery::VKCodeQuery(QWidget *parent)
	: QLineEdit(parent)
{
	setAttribute(Qt::WA_InputMethodEnabled, false); //禁用中文输入法
	pushKeyvector();	//把按键信息保存进容器里
}

VKCodeQuery::~VKCodeQuery()
{
}

bool VKCodeQuery::event(QEvent *event)
{
	if (event->type() == QEvent::KeyPress) 
	{
		KEY_INFORMATION Key_Information;
		QKeyEvent *ke = static_cast<QKeyEvent *>(event); //需要头文件支持
		std::size_t scanCode = ke->nativeVirtualKey();
		if (GetKeyInformation(Key_Information, scanCode)) {
			emit signal_keyInfo(Key_Information.Keyname, Key_Information.VK_Key, Key_Information.Key);
			return true;
		}
	}
	return QWidget::event(event);
}