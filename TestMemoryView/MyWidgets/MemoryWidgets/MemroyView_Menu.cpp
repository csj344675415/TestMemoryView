#include "MemoryView.h"
#include <QMenu>
#include <QAction>
#include <QShortcut>


#include <windows.h>


void MemoryView::initMenu()
{
	_pMenu = new QMenu(this);
	_pMenu->addAction(QStringLiteral("编辑地址"), this, &MemoryView::slot_editAddr);
	_pMenu->addAction(QStringLiteral("转到地址	Ctrl+G"), this, &MemoryView::slot_toAddr);
	QMenu* p5 = _pMenu->addMenu(QStringLiteral("转到选中数值"));
	//QAction* act = p5->addAction(QStringLiteral("首次进入	Ctrl+B"), this, &MemoryView::slot_intoAddr);
	//act->setToolTip(QStringLiteral("将选中的数值做为地址进行跳转"));
	QAction* act = p5->addAction(QStringLiteral("返回	-"), this, &MemoryView::slot_backspaceIntoAddr1);
	//act->setToolTip(QStringLiteral("返回跳转前的地址"));
	act = p5->addAction(QStringLiteral("进入	+"), this, &MemoryView::slot_backspaceIntoAddr2);
	//act->setToolTip(QStringLiteral("再次进入刚刚跳转的地址"));
	_pMenu->addSeparator();
	QMenu* p1 = _pMenu->addMenu(QStringLiteral("显示类型"));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("字节（十六进制）"), this, &MemoryView::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("字节（十进制）"), this, &MemoryView::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("2字节（十六进制）"), this, &MemoryView::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("2字节（十进制）"), this, &MemoryView::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("4字节（十六进制）"), this, &MemoryView::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("4字节（十进制）"), this, &MemoryView::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("8字节（十六进制）"), this, &MemoryView::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("8字节（十进制）"), this, &MemoryView::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("单浮点数"), this, &MemoryView::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("双浮点数"), this, &MemoryView::slot_dataType));
	QMenu* p2 = _pMenu->addMenu(QStringLiteral("文本编码"));
	_vecAction_TextType.append(p2->addAction(QStringLiteral("ASCII"), this, &MemoryView::slot_textType));
	_vecAction_TextType.append(p2->addAction(QStringLiteral("UTF-8"), this, &MemoryView::slot_textType));
	_vecAction_TextType.append(p2->addAction(QStringLiteral("UTF-16"), this, &MemoryView::slot_textType));
	QMenu* p4 = _pMenu->addMenu(QStringLiteral("分隔符"));
	_vecAction_divisionType.append(p4->addAction(QStringLiteral("2 字节"), this, &MemoryView::slot_divisionType));
	_vecAction_divisionType.append(p4->addAction(QStringLiteral("4 字节"), this, &MemoryView::slot_divisionType));
	_vecAction_divisionType.append(p4->addAction(QStringLiteral("8 字节"), this, &MemoryView::slot_divisionType));
	_pMenu->addAction(QStringLiteral("有符号"), this, &MemoryView::slot_symbol)->setCheckable(true);
	_pMenu->addSeparator();
	_pMenu->addAction(QStringLiteral("高亮时间"), this, &MemoryView::slot_setTime);
	_pMenu->addSeparator();
	QMenu* p3 = _pMenu->addMenu(QStringLiteral("更改页面保护"));
	p3->addAction(QStringLiteral("执行/读写"), this, &MemoryView::slot_protectType);
	p3->addAction(QStringLiteral("执行/读"), this, &MemoryView::slot_protectType);
	p3->addAction(QStringLiteral("读写"), this, &MemoryView::slot_protectType);
	p3->addAction(QStringLiteral("只读"), this, &MemoryView::slot_protectType);

	//
	for (auto t : _vecAction_DataType) {
		t->setCheckable(true);
	}
	_vecAction_DataType.at(0)->setChecked(true);

	//
	for (auto t : _vecAction_TextType) {
		t->setCheckable(true);
	}
	_vecAction_TextType.at(1)->setChecked(true);

	//
	for (auto t : _vecAction_divisionType) {
		t->setCheckable(true);
	}
	_vecAction_divisionType.at(1)->setChecked(true);
}

void MemoryView::slot_dataType()
{
	QObject *object = QObject::sender();
	if (object) {
		for (auto t : _vecAction_DataType) {
			t->setChecked(false);
		}

		QAction *pAction = qobject_cast<QAction *>(object);
		pAction->setChecked(true);

		QString text = pAction->text();
		if (text == QStringLiteral("字节（十六进制）")) {
			_dataType = e1ByteHex;
		}
		else if (text == QStringLiteral("字节（十进制）")) {
			_dataType = e1ByteDec;
		}
		else if (text == QStringLiteral("2字节（十六进制）")) {
			_dataType = e2ByteHex;
		}
		else if (text == QStringLiteral("2字节（十进制）")) {
			_dataType = e2ByteDec;
		}
		else if (text == QStringLiteral("4字节（十六进制）")) {
			_dataType = e4ByteHex;
		}
		else if (text == QStringLiteral("4字节（十进制）")) {
			_dataType = e4ByteDec;
		}
		else if (text == QStringLiteral("8字节（十六进制）")) {
			_dataType = e8ByteHex;
		}
		else if (text == QStringLiteral("8字节（十进制）")) {
			_dataType = e8ByteDec;
		}
		else if (text == QStringLiteral("单浮点数")) {
			_dataType = e4ByteFloat;
		}
		else if (text == QStringLiteral("双浮点数")) {
			_dataType = e8ByteDouble;
		}

		setItem();
		viewport()->update();
	}
}

void MemoryView::slot_textType()
{
	QObject *object = QObject::sender();
	if (object) {
		for (auto t : _vecAction_TextType) {
			t->setChecked(false);
		}

		QAction *pAction = qobject_cast<QAction *>(object);
		pAction->setChecked(true);

		QString text = pAction->text();
		if (text == QLatin1String("ASCII")) {
			_textType = eASCII;
		}
		else if (text == QLatin1String("UTF-8")) {
			_textType = eUTF8;
		}
		else if (text == QLatin1String("UTF-16")) {
			_textType = eUTF16;
		}

		setItem();
		viewport()->update();
	}
}

void MemoryView::slot_divisionType()
{
	QObject *object = QObject::sender();
	if (object) {
		for (auto t : _vecAction_divisionType) {
			t->setChecked(false);
		}

		QAction *pAction = qobject_cast<QAction *>(object);
		pAction->setChecked(true);

		QString text = pAction->text();
		if (text == QStringLiteral("2 字节")) {
			_divisionByte = 2;
		}
		else if (text == QStringLiteral("4 字节")) {
			_divisionByte = 4;
		}
		else if (text == QStringLiteral("8 字节")) {
			_divisionByte = 8;
		}
		viewport()->update();
	}
}
void MemoryView::slot_symbol()
{
	_bSymbol = !_bSymbol;
	setItem();
	viewport()->update();
}

void MemoryView::slot_protectType()
{
	QObject *object = QObject::sender();
	if (object) {
		if (_dataAddr1 < _mi.endAddr) {
			if (_mi.state != MEM_COMMIT) {
				return;
			}
		}
		else {
			if (_mi.state2 != MEM_COMMIT) {
				return;
			}
		}

		HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, false, _pid);

		// 内存信息
		MEMORY_BASIC_INFORMATION mbi{};
		memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
		::VirtualQueryEx(hProcess, (LPCVOID)_dataAddr1, &mbi, sizeof(mbi));

		DWORD lpflOldProtect;
		QAction *pAction = qobject_cast<QAction *>(object);
		QString text = pAction->text();
		if (text == QStringLiteral("执行/读写")) {
			VirtualProtectEx(hProcess, (LPVOID)mbi.BaseAddress, sizeof(mbi.RegionSize), PAGE_EXECUTE_READWRITE, &lpflOldProtect);
		}
		else if (text == QStringLiteral("执行/读")) {
			VirtualProtectEx(hProcess, (LPVOID)mbi.BaseAddress, sizeof(mbi.RegionSize), PAGE_EXECUTE_READ, &lpflOldProtect);
		}
		else if (text == QStringLiteral("读/写")) {
			VirtualProtectEx(hProcess, (LPVOID)mbi.BaseAddress, sizeof(mbi.RegionSize), PAGE_READWRITE, &lpflOldProtect);
		}
		else if (text == QStringLiteral("只读")) {
			VirtualProtectEx(hProcess, (LPVOID)mbi.BaseAddress, sizeof(mbi.RegionSize), PAGE_READONLY, &lpflOldProtect);
		}
		_mi.strProtect = text;
		viewport()->update();

		CloseHandle(hProcess);
	}
}

void MemoryView::slot_editAddr()
{
	_pEditor->show(_dataInfo, eEditorData);
}

void MemoryView::slot_intoAddr()
{
	if (_headAddr1 >= _firstAddr && _headAddr1 < _firstAddr + _maxRowCount * 0x10) {
		unsigned char* data = _buffer->data();
		unsigned long long intoAddr = 0;
#if _WIN64
		intoAddr = *(unsigned long long*) & data[_dataIndex1];
#else
		intoAddr = *(unsigned int*)&data[_dataIndex1];
#endif
		slot_changeAddr2(intoAddr);
	}
}

void MemoryView::slot_toAddr()
{
	if (_dataInfo.isValid() == false) {
		_dataInfo.pid = _pid;
		_dataInfo.beginAddr = _firstAddr;
	}
	_pEditor->show(_dataInfo, eToAddr);
}

void MemoryView::slot_setTime()
{
	_pEditor->show(_dataInfo, eUpdateTime);
}



void MemoryView::slot_changeData(DataInfo di)
{
	COMMADN_PARAMETER cp{};
	cp.addr = di.beginAddr;
	cp.pid = di.pid;
	cp.size = di.dataSize;
	cp.originalBuff = &_buffer->data()[_dataIndex1];
	cp.newBuff = di.data;

	_pUndoStack->push(new MemoryViewCommand(cp));
}

//void MemoryView::slot_changeAddr(unsigned long long addr)
//{
//	if (_bClicked) {
//		_addrStack.push(_headAddr1);
//	}
//	else {
//		_addrStack.push(_firstAddr);
//	}
//
//	_firstAddr = addr;
//	_mi = _buffer->changeAddr(_firstAddr);
//
//	if (_bClicked) {
//		_colIndex = 0;
//		_headAddr1 = _headAddr2 = _firstAddr;
//	}
//
//	viewport()->update();
//}

void MemoryView::slot_changeAddr2(unsigned long long addr)
{
	Into_Stack_Data is;
	is.colIndex = _colIndex;
	is.dataIndex = _dataIndex1;
	is.headAddr = _headAddr1;
	is.retAddr = _firstAddr;
	is.retSelect = _dataAddr1;
	is.intoAddr = addr;
	_intoStack1.push(is);

	//_intoStack2.clear();

	_colIndex = 0;
	_firstAddr = addr;
	_mi = _buffer->changeAddr(_firstAddr);

	viewport()->update();
}

void MemoryView::slot_changeTime(int ms)
{
	_buffer->changeTime(ms);
}
