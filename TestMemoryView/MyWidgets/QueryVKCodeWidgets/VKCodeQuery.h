#pragma once

#include <QLineEdit>



class VKCodeQuery : public QLineEdit
{
	Q_OBJECT

public:
	VKCodeQuery(QWidget *parent = Q_NULLPTR);
	~VKCodeQuery();

private:
	bool event(QEvent *event); //所有事件

public:

signals:
	void signal_keyInfo(char* keyName, char* vkName, int vkValue);
public:

};
