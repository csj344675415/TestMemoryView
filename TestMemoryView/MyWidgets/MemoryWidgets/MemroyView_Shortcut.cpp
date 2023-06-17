#include "MemoryView.h"
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QClipboard>

void MemoryView::initShortcut()
{
	// 快捷键	
	QShortcut* shortcut7 = new QShortcut(QKeySequence(tr("Ctrl+B")), this);				//查看数值地址
	connect(shortcut7, &QShortcut::activated, this, &MemoryView::slot_intoAddr);
	QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+G")), this);				//跳转
	connect(shortcut, &QShortcut::activated, this, &MemoryView::slot_toAddr);
	QShortcut* shortcut2 = new QShortcut(QKeySequence(tr("Ctrl+C")), this);				//复制
	connect(shortcut2, &QShortcut::activated, this, &MemoryView::slot_copy);
	QShortcut* shortcut3 = new QShortcut(QKeySequence(tr("Ctrl+V")), this);				//粘贴
	connect(shortcut3, &QShortcut::activated, this, &MemoryView::slot_paste);
	QShortcut* shortcut4 = new QShortcut(QKeySequence(tr("Ctrl+Z")), this);				//撤销
	connect(shortcut4, &QShortcut::activated, this, &MemoryView::slot_undo);
	QShortcut* shortcut5 = new QShortcut(QKeySequence(tr("Ctrl+Y")), this);				//重做
	connect(shortcut5, &QShortcut::activated, this, &MemoryView::slot_redo);
	QShortcut* shortcut6 = new QShortcut(QKeySequence(tr("Backspace")), this);			//返回上一个地址
	connect(shortcut6, &QShortcut::activated, this, &MemoryView::slot_backspaceAddr);
	QShortcut* shortcut8 = new QShortcut(QKeySequence(tr("-")), this);					//返回上一个into地址
	connect(shortcut8, &QShortcut::activated, this, &MemoryView::slot_backspaceIntoAddr1);
	QShortcut* shortcut9 = new QShortcut(QKeySequence(tr("+")), this);					//进入上一个into地址
	connect(shortcut9, &QShortcut::activated, this, &MemoryView::slot_backspaceIntoAddr2);
}

void MemoryView::slot_undo()
{
	_pUndoStack->undo();
}

void MemoryView::slot_redo()
{
	_pUndoStack->redo();
}

void MemoryView::slot_backspaceIntoAddr1()
{
	if (_intoStack1.size() > 0) {
		Into_Stack_Data is = _intoStack1.pop();
		//is.intoSelect = _dataAddr1;
		//_intoStack2.push(is);

		_colIndex = is.colIndex;
		_dataIndex1 = _dataIndex2 = is.dataIndex;
		_headAddr1 = _headAddr2 = is.headAddr;
		_dataAddr1 = _dataAddr2 = is.retSelect;
		_firstAddr = is.retAddr;
		_mi = _buffer->changeAddr(_firstAddr);

		viewport()->update();
	}
}
void MemoryView::slot_backspaceIntoAddr2()
{
	unsigned long long intoAddr = 0;
	if (_headAddr1 >= _firstAddr && _headAddr1 < _firstAddr + _maxRowCount * 0x10) {
		unsigned char* data = _buffer->data();
		//unsigned long long intoAddr = 0;
#if _WIN64
		intoAddr = *(unsigned long long*) & data[_dataIndex1];
#else
		intoAddr = *(unsigned int*)&data[_dataIndex1];
#endif

		Into_Stack_Data is;
		is.colIndex = _colIndex;
		is.dataIndex = _dataIndex1;
		is.headAddr = _headAddr1;
		is.retAddr = _firstAddr;
		is.retSelect = _dataAddr1;
		is.intoAddr = intoAddr;
		_intoStack1.push(is);


		_colIndex = 0;
		_firstAddr = intoAddr;
		_mi = _buffer->changeAddr(_firstAddr);

		viewport()->update();
	}
}

void MemoryView::slot_backspaceAddr()
{
	if (_addrStack.size() > 0) {
		_firstAddr = _addrStack.pop();
	}
	else if (_headAddr1 != 0 && qAbs((long long)(_firstAddr - _headAddr1)) > 400) {
		_firstAddr = _headAddr1;
	}
	else {
		return;
	}

	_mi = _buffer->changeAddr(_firstAddr);

	viewport()->update();
}

void MemoryView::slot_copy()
{
	unsigned char* data = _buffer->data();
	QString strData;
	if (_releaseArea == 1) {
		if (_dataIndex1 != _dataIndex2) {
			//1.先取得最大长度
			int maxLen = 0;
			for (int i = _itemIndex1; i <= _itemIndex2; ++i) {
				int len = _vecDataItem.at(i).text.length();
				maxLen = qMax(maxLen, len);
			}

			//2.对其复制数据
			//int nByte = 16 / _dataColumnCount;	//_dataStringWidth
			int n = _itemIndex1 % _dataColumnCount;
			for (int j = 0; j < n; ++j) {
				strData += QString("%1 ").arg(" ", maxLen);
			}

			for (int i = _itemIndex1; i <= _itemIndex2; ++i) {
				if ((i % _dataColumnCount) == 0) {
					strData += "\n";
				}
				strData += QString("%1 ").arg(_vecDataItem.at(i).text, maxLen);
			}
		}
		else {
			strData = _vecDataItem.at(_itemIndex1).text;
		}	
	}
	else if (_releaseArea == 2) {
		int size = _dataIndex2 - _dataIndex1 + 1;
		char* text = new char[size + 1]();
		memcpy(text, &data[_dataIndex1], size);

		bool isString = true;
		for (int i = 0; i < size; ++i) {
			int len = u8bytes(text[i]);
			if (len == 0) {
				isString = false;
				break;
			}
			else if (len == 1) {
				if (!isprint(text[i])) {
					isString = false;
					break;
				}
				else {
					strData += text[i];
				}
			}
			else if (len > 1) {
				char* sss = new char[len + 1]();
				memcpy(sss, &text[i], len);
				if (isUtf8Code(sss)) {
					strData += sss;
					delete[] sss;
				}
				else {
					isString = false;
					delete[] sss;
					break;
				}
				i = --i + len;
			}
		}

		if (!isString) {
			strData.clear();
			for (int i = _dataIndex1; i <= _dataIndex2; ++i) {
				strData += QString("%1 ").arg(data[i], 2, 16, QChar('0')).toUpper();
			}
		}

		delete[] text;
	}

	QClipboard *board = QApplication::clipboard();
	board->setText(strData);
}

void MemoryView::slot_paste()
{
	QClipboard *board = QApplication::clipboard();
	QString str = board->text();	//获取剪贴板数据

	if (str.endsWith(" ")) {
		//移除尾部空格
		str.remove(str.length() - 1, 1);
	}
	if (str.startsWith(" ")) {
		//移除头部空格
		str.remove(0, 1);
	}

	bool bbb;
	unsigned char* buff = nullptr;
	int size = 0;
	QRegExp rx;
	rx.setPattern(QString("^(-{1}[0-9]+)$"));	//设置正则表达式
	bbb = rx.exactMatch(str);
	if (bbb) {
		//PLOGD << "是10进制的负数";
		long long vvv = 0;
		vvv = str.toLongLong(&bbb, 10);
		if (bbb) {
			//PLOGD << "成功转为10进制数";
			if (vvv >= 0xFFFFFFFFFFFFFF80 && vvv <= 0x7F) {
				//PLOGD << "是 char 类型";
				size = 1;
				buff = new unsigned char[size]();
				memcpy(buff, (unsigned char*)&vvv, size);
			}
			else if (vvv >= 0xFFFFFFFFFFFF8000 && vvv <= 0x7FFF) {
				//PLOGD << "是 short 类型";
				size = 2;
				buff = new unsigned char[size]();
				memcpy(buff, (unsigned char*)&vvv, size);
			}
			else if (vvv >= 0xFFFFFFFF80000000 && vvv <= 0x7FFFFFFF) {
				//PLOGD << "是 int 类型";
				size = 4;
				buff = new unsigned char[size]();
				memcpy(buff, (unsigned char*)&vvv, size);
			}
			else {
				//PLOGD << "是 long long 类型";
				size = 8;
				buff = new unsigned char[size]();
				memcpy(buff, (unsigned char*)&vvv, size);
			}
		}
		else {
			//PLOGD << "是字符串";
			QByteArray ba = str.toUtf8();
			int len = ba.length();
			size = len;
			buff = new unsigned char[size]();
			memcpy(buff, ba.data(), size);
		}
	}
	else {
		rx.setPattern(QString("^(0x)?[A-Fa-f0-9]+$"));
		bbb = rx.exactMatch(str);
		if (bbb) {
			//PLOGD << "是16进制数还是字节数组？";
			int len = str.length();
			bbb = str.startsWith("0x");
			if (bbb) len -= 2;
			if (len > 16) {
				//PLOGD << "是字节数组";
				int len = str.length();
				if (len % 2 != 0) {
					str = "0" + str;
					len += 1;
				}
				size = len;
				buff = new unsigned char[size]();
				for (int i = 0; i < size / 2; ++i) {
					buff[i] = str.mid(i * 2, 2).toUInt(&bbb, 16);
				}
			}
			else if (len > 8) {
				//PLOGD << "是 unsigned long long 类型";
				size = 8;
				buff = new unsigned char[size]();
				unsigned long long value = str.toULongLong(&bbb, 16);
				memcpy(buff, (unsigned char*)&value, size);
			}
			else if (len > 4) {
				//PLOGD << "是 unsigned int 类型";
				size = 4;
				buff = new unsigned char[size]();
				unsigned long long value = str.toULongLong(&bbb, 16);
				memcpy(buff, (unsigned char*)&value, size);
			}
			else if (len > 2) {
				//PLOGD << "是 unsigned short 类型";
				size = 2;
				buff = new unsigned char[size]();
				unsigned long long value = str.toULongLong(&bbb, 16);
				memcpy(buff, (unsigned char*)&value, size);
			}
			else {
				//PLOGD << "是 unsigned char 类型";
				size = 1;
				buff = new unsigned char[size]();
				unsigned long long value = str.toULongLong(&bbb, 16);
				memcpy(buff, (unsigned char*)&value, size);
			}
		}
		else {
			rx.setPattern(QString("^(-?\\d+(\\.{1}\\d+)?)$"));
			bbb = rx.exactMatch(str);
			if (bbb) {
				//PLOGD << "是浮点数";
				float vvv = 0.f;
				vvv = str.toFloat(&bbb);
				if (bbb) {
					//PLOGD << "成功转为单浮点数";
					size = 4;
					buff = new unsigned char[size]();
					memcpy(buff, (unsigned char*)&vvv, size);
				}
				else {
					double vvv2 = 0.f;
					vvv2 = str.toDouble(&bbb);
					if (bbb) {
						//PLOGD << "成功转为双浮点数";
						size = 8;
						buff = new unsigned char[size]();
						memcpy(buff, (unsigned char*)&vvv2, size);
					}
					else {
						//PLOGD << "是字符串";
						QByteArray ba = str.toUtf8();
						int len = ba.length();
						size = len;
						buff = new unsigned char[size]();
						memcpy(buff, ba.data(), size);
					}
				}
			}
			else {
				//PLOGD << "是字节数组还是字符串？";
				QString strTemp = str;
				strTemp.remove(QRegExp("\\s"));
				rx.setPattern(QString("^[A-Fa-f0-9]+$"));
				bbb = rx.exactMatch(strTemp);
				if (!bbb) {
					//PLOGD << "是字符串";
					QByteArray ba = str.toUtf8();
					int len = ba.length();
					size = len;
					buff = new unsigned char[size]();
					memcpy(buff, ba.data(), size);
				}
				else {
					unsigned char tttt[256]{};
					QStringList strList = str.split(" ");
					if (strList.size() > 0) {
						int maxLen = strList.at(0).length();
						if (maxLen % 2 != 0) {	//长度不是2的倍数视为字符串
							//PLOGD << "是字符串";
							QByteArray ba = str.toUtf8();
							int len = ba.length();
							size = len;
							buff = new unsigned char[size]();
							memcpy(buff, ba.data(), size);
						}
						else {
							//PLOGD << "是字节数组？";
							for (int i = 0; i < strList.size(); ++i) {
								int len = strList.at(i).length();
								if (len != maxLen) {	//长度不一致视为字符串，否则为16进制数组
									//PLOGD << "是字符串";
									QByteArray ba = str.toUtf8();
									int len = ba.length();
									size = len;
									buff = new unsigned char[size]();
									memcpy(buff, ba.data(), size);
									break;
								}
								else {
									size += len / 2;
									unsigned long long value = 0;
									if (len == 2) {
										//1字节
										value = strList.at(i).toULongLong(&bbb, 16);
										memcpy(&tttt[i * (len / 2)], (unsigned char*)&value, len / 2);
									}
									else if (len == 4) {
										//2字节
										value = strList.at(i).toULongLong(&bbb, 16);
										memcpy(&tttt[i * (len / 2)], (unsigned char*)&value, len / 2);
									}
									else if (len == 8) {
										//4字节
										value = strList.at(i).toULongLong(&bbb, 16);
										memcpy(&tttt[i * (len / 2)], (unsigned char*)&value, len / 2);
									}
									else if (len == 16) {
										//8字节
										value = strList.at(i).toULongLong(&bbb, 16);
										memcpy(&tttt[i * (len / 2)], (unsigned char*)&value, len / 2);
									}
								}
								buff = new unsigned char[size]();
								memcpy(buff, tttt, size);
							}
						}
					}
				}
			}
		}
	}

	COMMADN_PARAMETER cp{};
	cp.addr = _dataAddr1;
	cp.pid = _pid;
	cp.size = size;
	cp.originalBuff = &_buffer->data()[_dataIndex1];
	cp.newBuff = buff;

	_pUndoStack->push(new MemoryViewCommand(cp));

	delete[] buff;
}

