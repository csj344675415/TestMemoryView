#include "MemoryView.h"
#include <QMenu>
#include <QAction>
#include <QShortcut>
#include <QClipboard>

void MemoryView::initShortcut()
{
	// ��ݼ�	
	QShortcut* shortcut7 = new QShortcut(QKeySequence(tr("Ctrl+B")), this);				//�鿴��ֵ��ַ
	connect(shortcut7, &QShortcut::activated, this, &MemoryView::slot_intoAddr);
	QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+G")), this);				//��ת
	connect(shortcut, &QShortcut::activated, this, &MemoryView::slot_toAddr);
	QShortcut* shortcut2 = new QShortcut(QKeySequence(tr("Ctrl+C")), this);				//����
	connect(shortcut2, &QShortcut::activated, this, &MemoryView::slot_copy);
	QShortcut* shortcut3 = new QShortcut(QKeySequence(tr("Ctrl+V")), this);				//ճ��
	connect(shortcut3, &QShortcut::activated, this, &MemoryView::slot_paste);
	QShortcut* shortcut4 = new QShortcut(QKeySequence(tr("Ctrl+Z")), this);				//����
	connect(shortcut4, &QShortcut::activated, this, &MemoryView::slot_undo);
	QShortcut* shortcut5 = new QShortcut(QKeySequence(tr("Ctrl+Y")), this);				//����
	connect(shortcut5, &QShortcut::activated, this, &MemoryView::slot_redo);
	QShortcut* shortcut6 = new QShortcut(QKeySequence(tr("Backspace")), this);			//������һ����ַ
	connect(shortcut6, &QShortcut::activated, this, &MemoryView::slot_backspaceAddr);
	QShortcut* shortcut8 = new QShortcut(QKeySequence(tr("-")), this);					//������һ��into��ַ
	connect(shortcut8, &QShortcut::activated, this, &MemoryView::slot_backspaceIntoAddr1);
	QShortcut* shortcut9 = new QShortcut(QKeySequence(tr("+")), this);					//������һ��into��ַ
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
			//1.��ȡ����󳤶�
			int maxLen = 0;
			for (int i = _itemIndex1; i <= _itemIndex2; ++i) {
				int len = _vecDataItem.at(i).text.length();
				maxLen = qMax(maxLen, len);
			}

			//2.���临������
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
	QString str = board->text();	//��ȡ����������

	if (str.endsWith(" ")) {
		//�Ƴ�β���ո�
		str.remove(str.length() - 1, 1);
	}
	if (str.startsWith(" ")) {
		//�Ƴ�ͷ���ո�
		str.remove(0, 1);
	}

	bool bbb;
	unsigned char* buff = nullptr;
	int size = 0;
	QRegExp rx;
	rx.setPattern(QString("^(-{1}[0-9]+)$"));	//����������ʽ
	bbb = rx.exactMatch(str);
	if (bbb) {
		//PLOGD << "��10���Ƶĸ���";
		long long vvv = 0;
		vvv = str.toLongLong(&bbb, 10);
		if (bbb) {
			//PLOGD << "�ɹ�תΪ10������";
			if (vvv >= 0xFFFFFFFFFFFFFF80 && vvv <= 0x7F) {
				//PLOGD << "�� char ����";
				size = 1;
				buff = new unsigned char[size]();
				memcpy(buff, (unsigned char*)&vvv, size);
			}
			else if (vvv >= 0xFFFFFFFFFFFF8000 && vvv <= 0x7FFF) {
				//PLOGD << "�� short ����";
				size = 2;
				buff = new unsigned char[size]();
				memcpy(buff, (unsigned char*)&vvv, size);
			}
			else if (vvv >= 0xFFFFFFFF80000000 && vvv <= 0x7FFFFFFF) {
				//PLOGD << "�� int ����";
				size = 4;
				buff = new unsigned char[size]();
				memcpy(buff, (unsigned char*)&vvv, size);
			}
			else {
				//PLOGD << "�� long long ����";
				size = 8;
				buff = new unsigned char[size]();
				memcpy(buff, (unsigned char*)&vvv, size);
			}
		}
		else {
			//PLOGD << "���ַ���";
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
			//PLOGD << "��16�����������ֽ����飿";
			int len = str.length();
			bbb = str.startsWith("0x");
			if (bbb) len -= 2;
			if (len > 16) {
				//PLOGD << "���ֽ�����";
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
				//PLOGD << "�� unsigned long long ����";
				size = 8;
				buff = new unsigned char[size]();
				unsigned long long value = str.toULongLong(&bbb, 16);
				memcpy(buff, (unsigned char*)&value, size);
			}
			else if (len > 4) {
				//PLOGD << "�� unsigned int ����";
				size = 4;
				buff = new unsigned char[size]();
				unsigned long long value = str.toULongLong(&bbb, 16);
				memcpy(buff, (unsigned char*)&value, size);
			}
			else if (len > 2) {
				//PLOGD << "�� unsigned short ����";
				size = 2;
				buff = new unsigned char[size]();
				unsigned long long value = str.toULongLong(&bbb, 16);
				memcpy(buff, (unsigned char*)&value, size);
			}
			else {
				//PLOGD << "�� unsigned char ����";
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
				//PLOGD << "�Ǹ�����";
				float vvv = 0.f;
				vvv = str.toFloat(&bbb);
				if (bbb) {
					//PLOGD << "�ɹ�תΪ��������";
					size = 4;
					buff = new unsigned char[size]();
					memcpy(buff, (unsigned char*)&vvv, size);
				}
				else {
					double vvv2 = 0.f;
					vvv2 = str.toDouble(&bbb);
					if (bbb) {
						//PLOGD << "�ɹ�תΪ˫������";
						size = 8;
						buff = new unsigned char[size]();
						memcpy(buff, (unsigned char*)&vvv2, size);
					}
					else {
						//PLOGD << "���ַ���";
						QByteArray ba = str.toUtf8();
						int len = ba.length();
						size = len;
						buff = new unsigned char[size]();
						memcpy(buff, ba.data(), size);
					}
				}
			}
			else {
				//PLOGD << "���ֽ����黹���ַ�����";
				QString strTemp = str;
				strTemp.remove(QRegExp("\\s"));
				rx.setPattern(QString("^[A-Fa-f0-9]+$"));
				bbb = rx.exactMatch(strTemp);
				if (!bbb) {
					//PLOGD << "���ַ���";
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
						if (maxLen % 2 != 0) {	//���Ȳ���2�ı�����Ϊ�ַ���
							//PLOGD << "���ַ���";
							QByteArray ba = str.toUtf8();
							int len = ba.length();
							size = len;
							buff = new unsigned char[size]();
							memcpy(buff, ba.data(), size);
						}
						else {
							//PLOGD << "���ֽ����飿";
							for (int i = 0; i < strList.size(); ++i) {
								int len = strList.at(i).length();
								if (len != maxLen) {	//���Ȳ�һ����Ϊ�ַ���������Ϊ16��������
									//PLOGD << "���ַ���";
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
										//1�ֽ�
										value = strList.at(i).toULongLong(&bbb, 16);
										memcpy(&tttt[i * (len / 2)], (unsigned char*)&value, len / 2);
									}
									else if (len == 4) {
										//2�ֽ�
										value = strList.at(i).toULongLong(&bbb, 16);
										memcpy(&tttt[i * (len / 2)], (unsigned char*)&value, len / 2);
									}
									else if (len == 8) {
										//4�ֽ�
										value = strList.at(i).toULongLong(&bbb, 16);
										memcpy(&tttt[i * (len / 2)], (unsigned char*)&value, len / 2);
									}
									else if (len == 16) {
										//8�ֽ�
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

