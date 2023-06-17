#include "HexEdit.h"
#include <QPainter>
#include <QPaintEvent>
#include <QScrollBar>
#include <QMenu>
#include <QTimerEvent>
#include <QShortcut>
#include <QClipboard>
#include <QUndoStack>


#include "ReuseCode.hpp"
extern ReuseCode *prc;

#include "WriteCommand.h"


HexEdit::HexEdit(QWidget *parent)
	: QAbstractScrollArea(parent)
{
	ui.setupUi(this);
	setMouseTracking(true);
	setFont(QFont("Courier", 10));
	_originalFont = this->font();		//ԭʼ����
	_boldFont = _originalFont;			//�Ӵ�����
	_boldFont.setWeight(QFont::Black);
	_pFontMetrics = new QFontMetrics(this->font());
	
	init();
	initMenu();
	_updateViewItem();
}

HexEdit::~HexEdit()
{
	// �˳��߳�
	if (_t) {
		if (_t->joinable()) {
			_bStrat_t = false;
			_t->join();
			delete _t;
			_t = nullptr;
		}
	}

	// �رն�ʱ��
	killTimer(_nTimerID);
	_nTimerID = -1;
	killTimer(_nTimerID2);
	_nTimerID2 = -1;

	delete _pFontMetrics;
	delete _pEditorWidgets;
	delete _pUndoStack;
}

void HexEdit::init()
{
	verticalScrollBar()->setRange(0, 0x7FFFFFFF);
	verticalScrollBar()->setPageStep(0x10 * 3);	//���»������ͷ��ť֮�������ʱ�ƶ��Ĳ���
	verticalScrollBar()->setSingleStep(0x10);	//���¼�ͷ��ťʱ�ƶ��Ĳ���
	_middleValue = verticalScrollBar()->maximum() / 2;
	verticalScrollBar()->setValue(_middleValue);

	connect(verticalScrollBar(), SIGNAL(sliderPressed()), this, SLOT(slot_sliderPressed()));	//���»���
	connect(verticalScrollBar(), SIGNAL(sliderReleased()), this, SLOT(slot_sliderReleased()));	//�ͷŻ���
	connect(verticalScrollBar(), SIGNAL(actionTriggered(int)), this, SLOT(slot_actionTriggered(int)));	//��������������

	connect(this, &HexEdit::signal_updateHexEditView, this, [this] {viewport()->update(); });

	_pEditorWidgets = new EditorWidgets;
	connect(_pEditorWidgets, &EditorWidgets::signal_changeData, this, &HexEdit::slot_changeData);
	connect(_pEditorWidgets, &EditorWidgets::signal_toAddr, this, &HexEdit::slot_gotoAddr);
	connect(_pEditorWidgets, &EditorWidgets::signal_setTime, this, &HexEdit::slot_setTime);

	_dataType = e1ByteHex;
	_textType = eASCII;

	_pUndoStack = new QUndoStack(this);
}

void HexEdit::initMenu()
{
	_pMenu = new QMenu(this);
	_pMenu->addAction(QStringLiteral("ת����ַ	Ctrl+G"), this, &HexEdit::slot_toAddr);
	_pMenu->addSeparator();
	QMenu* p1 = _pMenu->addMenu(QStringLiteral("��ʾ����"));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("�ֽڣ�ʮ�����ƣ�"), this, &HexEdit::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("�ֽڣ�ʮ���ƣ�"), this, &HexEdit::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("2�ֽڣ�ʮ�����ƣ�"), this, &HexEdit::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("2�ֽڣ�ʮ���ƣ�"), this, &HexEdit::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("4�ֽڣ�ʮ�����ƣ�"), this, &HexEdit::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("4�ֽڣ�ʮ���ƣ�"), this, &HexEdit::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("8�ֽڣ�ʮ�����ƣ�"), this, &HexEdit::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("8�ֽڣ�ʮ���ƣ�"), this, &HexEdit::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("��������"), this, &HexEdit::slot_dataType));
	_vecAction_DataType.append(p1->addAction(QStringLiteral("˫������"), this, &HexEdit::slot_dataType));
	QMenu* p2 = _pMenu->addMenu(QStringLiteral("�ı�����"));
	_vecAction_TextType.append(p2->addAction(QStringLiteral("ASCII"), this, &HexEdit::slot_textType));
	_vecAction_TextType.append(p2->addAction(QStringLiteral("UTF-8"), this, &HexEdit::slot_textType));
	_vecAction_TextType.append(p2->addAction(QStringLiteral("UTF-16"), this, &HexEdit::slot_textType));
	QMenu* p4 = _pMenu->addMenu(QStringLiteral("�ָ���"));
	_vecAction_divisionType.append(p4->addAction(QStringLiteral("2 �ֽ�"), this, &HexEdit::slot_divisionType));
	_vecAction_divisionType.append(p4->addAction(QStringLiteral("4 �ֽ�"), this, &HexEdit::slot_divisionType));
	_vecAction_divisionType.append(p4->addAction(QStringLiteral("8 �ֽ�"), this, &HexEdit::slot_divisionType));
	_pMenu->addAction(QStringLiteral("�з���"), this, &HexEdit::slot_symbol)->setCheckable(true);
	_pMenu->addSeparator();
	_pMenu->addAction(QStringLiteral("����ʱ��"), this, &HexEdit::slot_changeTime);
	_pMenu->addSeparator();
	QMenu* p3 = _pMenu->addMenu(QStringLiteral("����ҳ�汣��"));
	p3->addAction(QStringLiteral("ִ��/��д"), this, &HexEdit::slot_protectType);
	p3->addAction(QStringLiteral("ִ��/��"), this, &HexEdit::slot_protectType);
	p3->addAction(QStringLiteral("��д"), this, &HexEdit::slot_protectType);
	p3->addAction(QStringLiteral("ֻ��"), this, &HexEdit::slot_protectType);

	// ��ݼ�	
	QShortcut* shortcut = new QShortcut(QKeySequence(tr("Ctrl+G")), this);
	connect(shortcut, &QShortcut::activated, this, &HexEdit::slot_toAddr);
	QShortcut* shortcut2 = new QShortcut(QKeySequence(tr("Ctrl+C")), this);
	connect(shortcut2, &QShortcut::activated, this, &HexEdit::slot_copy);
	QShortcut* shortcut3 = new QShortcut(QKeySequence(tr("Ctrl+V")), this);
	connect(shortcut3, &QShortcut::activated, this, &HexEdit::slot_paste);
	QShortcut* shortcut4 = new QShortcut(QKeySequence(tr("Ctrl+Z")), this);	//����
	connect(shortcut4, &QShortcut::activated, this, &HexEdit::slot_undo);
	QShortcut* shortcut5 = new QShortcut(QKeySequence(tr("Ctrl+Y")), this);	//����
	connect(shortcut5, &QShortcut::activated, this, &HexEdit::slot_redo);
	QShortcut* shortcut6 = new QShortcut(QKeySequence(tr("Backspace")), this);	//����
	connect(shortcut6, &QShortcut::activated, this, &HexEdit::slot_backspaceAddr);

	//
	for (auto t : _vecAction_DataType) {
		t->setCheckable(true);
	}
	_vecAction_DataType.at(0)->setChecked(true);

	//
	for (auto t : _vecAction_TextType) {
		t->setCheckable(true);
	}
	_vecAction_TextType.at(0)->setChecked(true);

	//
	for (auto t : _vecAction_divisionType) {
		t->setCheckable(true);
	}
	_vecAction_divisionType.at(1)->setChecked(true);
}



void HexEdit::_updateViewItem()
{
	_rowHeight = _pFontMetrics->height();			//�и�
	_maxRows = this->rect().height() / _rowHeight;	//����
	_maxAddrWidth = _getStringWidth(eAddr_Width);
	_maxDataWidth = _getStringWidth(eData_Width);
	_maxTextWidth = _getStringWidth(eText_Width);
	//_dataCols	��ͨ�� getStringWidth ����ȡ��
	//_textCols ��ͨ�� getStringWidth ����ȡ��
	_rtAddrItemWidth = _maxAddrWidth + 8;
	_rtDataItemWidth = getDataItemWidth();
	_rtTextItemWidth = _maxTextWidth;
	


	_rtMemInfoArea.setRect(this->rect().x(), this->rect().y(), 1920, _rowHeight + 4);
	_rtOffsetArea.setRect(this->rect().x(), _rtMemInfoArea.bottom(), 1920, _rowHeight);
	_rtAddrArea.setRect(this->rect().x(), _rtOffsetArea.bottom(), _rtAddrItemWidth, this->rect().height());
	_rtDataArea.setRect(_rtAddrArea.right() + 4, _rtOffsetArea.bottom(), _rtDataItemWidth * _dataCols, this->rect().height());
	_rtTextArea.setRect(_rtDataArea.right() + 4, _rtOffsetArea.bottom(), _rtTextItemWidth * _textCols, this->rect().height());


	_vecAddrItem.clear();
	_vecDataItem.clear();
	_vecTextItem.clear();
	HexEditItem rt1{}, rt2{}, rt3{};
	for (int i = 0; i < _maxRows; ++i) {
		rt1.rect.setRect(_rtAddrArea.x(), (_rtAddrArea.y() + 2) + i * _rowHeight, _rtAddrArea.width(), _rowHeight);
		_vecAddrItem.append(rt1);

		for (int j = 0; j < _dataCols; ++j) {
			rt2.rect.setRect(_rtDataArea.x() + j * _rtDataItemWidth, rt1.rect.y(), _rtDataItemWidth, _rowHeight);
			_vecDataItem.append(rt2);
		}

		for (int k = 0; k < _textCols; ++k) {
			rt3.rect.setRect(_rtTextArea.x() + k * _rtTextItemWidth, rt1.rect.y(), _rtTextItemWidth, _rowHeight);
			_vecTextItem.append(rt3);
		}
	}

	viewport()->update();
}


int HexEdit::_getStringWidth(int id)
{
	int w = 0;
	if (id == eAddr_Width) {
#if _WIN64
		w = _pFontMetrics->width(QStringLiteral("FFFFFFFFFFFFFFFF"));
#else
		w = _pFontMetrics->width(QStringLiteral("FFFFFFFF"));
#endif
	}
	else if (id == eData_Width) {
		switch (_dataType)
		{
		case e1ByteHex:_dataCols = 16; w = _pFontMetrics->width(QStringLiteral("FF")); break;	//��Χһ�������ַ����Ŀ��	+8�����Ҽ��4������
		case e1ByteDec:_dataCols = 16; w = _pFontMetrics->width(QStringLiteral("-127")); break;
		case e2ByteHex:_dataCols = 8; w = _pFontMetrics->width(QStringLiteral("FFFF")); break;
		case e2ByteDec:_dataCols = 8; w = _pFontMetrics->width(QStringLiteral("-32768")); break;
		case e4ByteHex:_dataCols = 4; w = _pFontMetrics->width(QStringLiteral("FFFFFFFF")); break;
		case e4ByteDec:_dataCols = 4; w = _pFontMetrics->width(QStringLiteral("-2147483648")); break;
		case e8ByteHex:_dataCols = 2; w = _pFontMetrics->width(QStringLiteral("FFFFFFFFFFFFFFFF")); break;
		case e8ByteDec:_dataCols = 2; w = _pFontMetrics->width(QStringLiteral("-9223372036854775808")); break;
		case e4ByteFloat:_dataCols = 4; w = _pFontMetrics->width(QStringLiteral("-2147483648")); break; break;
		case e8ByteDouble:_dataCols = 2; w = _pFontMetrics->width(QStringLiteral("-9223372036854775808")); break;
		}
	}
	else if (id == eText_Width) {
		switch (_textType)
		{
		case eASCII:_textCols = 16; w = _pFontMetrics->width(QStringLiteral("F"));	break;
		case eUTF8:_textCols = 16; w = _pFontMetrics->width(QStringLiteral("F")); break;
		case eUTF16:_textCols = 16; w = _pFontMetrics->width(QStringLiteral("F")); break;
		}
	}

	return w;
}

int HexEdit::getDataItemWidth()
{
	int w = _maxDataWidth;
	int w2 = _pFontMetrics->width(QStringLiteral("F"));
	if ((w + 8) * _dataCols <= (w2 * 2 + 8) * 16) {
		w = ((w2 * 2 + 8) * 16) / _dataCols;
	}
	else {
		if (_bSymbol) {
			w = w + 8 - 2;	//��CE��ͬ��-8
		}
		else {
			w = w + 8 - 6;	//��CE��ͬ��-8
		}
	}
	return w;
}




void HexEdit::resizeEvent(QResizeEvent *event)
{
	int rows = (this->rect().height()) / (_rowHeight);
	_maxBytes = rows * 16;
	if (rows != _maxRows) {
		_updateViewItem();
	}
}

void HexEdit::paintEvent(QPaintEvent *event)
{
	QPainter painter(viewport());

	// ���Ʊ�������
	painter.fillRect(_rtMemInfoArea, this->palette().alternateBase().color());		
	painter.fillRect(_rtOffsetArea, this->palette().alternateBase().color());		
	painter.fillRect(_rtAddrArea, this->palette().alternateBase().color());			

	// ��������ָ���
	painter.setPen(Qt::lightGray);
	painter.drawLine(_rtAddrArea.right(), _rtOffsetArea.bottom(), _rtAddrArea.right(), event->rect().bottom());	//����
	painter.setPen(Qt::black);
	painter.drawLine(0, _rtOffsetArea.bottom(), event->rect().right(), _rtOffsetArea.bottom());	//����

	// 
	painter.setPen(Qt::black);
	_AddrHeadItem.rect = _vecAddrItem.at(0).rect;
	_AddrHeadItem.rect.setRect(_AddrHeadItem.rect.x() + 4, _rtOffsetArea.y(), _AddrHeadItem.rect.width(), _AddrHeadItem.rect.height());
	painter.drawText(_AddrHeadItem.rect, Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("��ַ"));

	// ���Item
	drawMemoryInfo(painter);	//�����ڴ���Ϣ
	drawAddress(painter);		//���Ƶ�ַ
	drawData(painter);			//��������
	drawTextData(painter);		//�����ı�����
	drawDataHead(painter);		//�������������ͷ
	drawTextDataHead(painter);	//�����ı������ͷ
	drawDivisionLine(painter);	//���Ʒָ��

}


#define BaseColor "#6E6E6E"		//��ַ��ƫ�Ƶ�������ɫ		��ɫ
#define BaseColor2 "#008036"	//���ݵ�������ɫ			//��ɫ
#define SelectColor Qt::black			//ѡ������ʱ��ַ��ƫ�Ƶ�������ɫ	��ɫ
#define SelectColor2 Qt::red			//ѡ������ʱ���ݵ�������ɫ			��ɫ

void HexEdit::drawDivisionLine(QPainter &painter)
{
	painter.save();

	QRect rtTemp;
	int x = _firstAddr % _divisionByte;	//��ַ����
	if (_dataType == e1ByteHex || _dataType == e1ByteDec) {
		for (int i = 0; i < 16; ++i) {
			if (i && (i + x - _divisionByte) % _divisionByte == 0) {
				// ��һ����ɫ�ָ���
				painter.setPen(QColor(230, 230, 0));
				rtTemp = _vecDataItem.at(i).rect;
				painter.drawLine(rtTemp.x() - 4, _ProtectItem.rect.bottom(), rtTemp.x() - 4, 1080);
			}
		}
	}

	painter.restore();
}


void HexEdit::drawDataHead(QPainter &painter)
{
	painter.save();

	painter.setPen(Qt::black);

	QRect rtTemp;
	QString str;
	int n = 16 / _dataCols;
	for (int i = 0; i < _dataCols; ++i) {
		str = QString("%1").arg((unsigned char)(i*n), 2, 16, QChar('0')).toUpper();

		if (_bClicked && (i*n) == _colIndex) {

			//if (_dataType == e1ByteHex) {
			//	// ������ı���
			//	ttt = _vecDataItem.at(i);
			//	ttt.setRect(ttt.x(), _rtOffsetArea.bottom() + 1, _pFontMetrics->width(str), event->rect().height());
			//	painter.fillRect(ttt, QColor(245, 245, 245));
			//}6E6E6E

			painter.setPen(QColor("#3399FF"));
			painter.setFont(_boldFont);
		}
		else {
			painter.setPen(Qt::black);
			painter.setFont(_originalFont);
		}


		rtTemp = _vecDataItem.at(i).rect;
		rtTemp.setRect(rtTemp.x(), _rtOffsetArea.y(), rtTemp.width(), rtTemp.height());
		painter.drawText(rtTemp, Qt::AlignLeft | Qt::AlignVCenter, str);
	}

	painter.restore();
}

void HexEdit::drawTextDataHead(QPainter &painter)
{
	painter.save();

	QRect rtTemp;
	QString str;
	int n = 16 / _textCols;
	for (int i = 0; i < _textCols; ++i) {
		if (_bClicked && (i*n) == _colIndex) {
			painter.setPen(QColor("#3399FF"));
			painter.setFont(_boldFont);
		}
		else {
			painter.setPen(Qt::black);
			painter.setFont(_originalFont);
		}

		str = QString("%1").arg(i*n, 1, 16, QChar('0')).toUpper();
		rtTemp = _vecTextItem.at(i).rect;
		rtTemp.setRect(rtTemp.x(), _rtOffsetArea.y(), rtTemp.width(), rtTemp.height());
		painter.drawText(rtTemp, Qt::AlignLeft | Qt::AlignVCenter, str);
	}

	painter.restore();
}

void HexEdit::drawMemoryInfo(QPainter &painter)
{
	painter.save();

	// �����ڴ������Ϣ
	QString str = QString(u8"������%1").arg(_mi.strProtect);
	int w = _pFontMetrics->width(str) + 16;
	_ProtectItem.rect.setRect(this->rect().x() + 4, this->rect().y() + 2, w, _rowHeight);
	painter.setPen(Qt::black);
	painter.drawText(_ProtectItem.rect, Qt::AlignLeft | Qt::AlignVCenter, str);

	str = QStringLiteral("AllocationBase=") + QString(u8"%1").arg(_mi.allocationBase, 0, 16, QChar('0')).toUpper();
	w = _pFontMetrics->width(str) + 8;
	_AllocationBaseAddrItem.rect.setRect(_ProtectItem.rect.right(), _ProtectItem.rect.y(), w, _rowHeight);
	painter.drawText(_AllocationBaseAddrItem.rect, Qt::AlignLeft | Qt::AlignVCenter, str);

	str = QString(u8"��ַ=%1").arg(_mi.beginAddr, 0, 16, QChar('0')).toUpper();
	w = _pFontMetrics->width(str) + 8;
	_BaseAddrItem.rect.setRect(_AllocationBaseAddrItem.rect.right(), _AllocationBaseAddrItem.rect.y(), w, _rowHeight);
	painter.drawText(_BaseAddrItem.rect, Qt::AlignLeft | Qt::AlignVCenter, str);

	str = QString(u8"��С=%1").arg(_mi.size, 0, 16, QChar('0')).toUpper();
	w = _pFontMetrics->width(str) + 8;
	_AddrSizeItem.rect.setRect(_BaseAddrItem.rect.right(), _BaseAddrItem.rect.y(), w, _rowHeight);
	painter.drawText(_AddrSizeItem.rect, Qt::AlignLeft | Qt::AlignVCenter, str);

	str = QString(u8"%1").arg(_mi.moduleName);
	w = _pFontMetrics->width(str) + 8;
	_ModuleNameItem.rect.setRect(_AddrSizeItem.rect.right(), _AddrSizeItem.rect.y(), w, _rowHeight);
	painter.drawText(_ModuleNameItem.rect, Qt::AlignLeft | Qt::AlignVCenter, str);

	painter.restore();
}

void HexEdit::drawAddress(QPainter &painter)
{
	painter.save();

	painter.setPen(QColor("#6E6E6E"));

	for (int i = 0; i < _maxRows; ++i) {
		// ȡ�õ�ַ����
		unsigned long long addr = _firstAddr + i * 0x10;
#if _WIN64
		_vecAddrItem[i].text.swap(QString("%1").arg(addr, 16, 16, QChar('0')).toUpper());
#else
		_vecAddrItem[i].text.swap(QString("%1").arg(addr, 8, 16, QChar('0')).toUpper());
#endif

		if (_bClicked && addr == _headAddr1) {
			QRect rtrt = _vecAddrItem.at(i).rect;
			rtrt.setRect(rtrt.right() + 1, rtrt.y(), 1920, rtrt.height());
			painter.fillRect(rtrt, QColor(245, 245, 245));
		}


		if (addr >= _headAddr1 && addr <= _headAddr2) {
			painter.setPen(QColor("#3399FF"));
			painter.setFont(_boldFont);
		}
		else {
			painter.setPen(Qt::black);
			painter.setFont(_originalFont);
		}

		// ���Ƶ�ַ �� ��ԭ����
		painter.drawText(_vecAddrItem.at(i).rect, Qt::AlignCenter, _vecAddrItem.at(i).text);
		painter.setFont(_originalFont);
	}

	painter.restore();
}




void HexEdit::drawData(QPainter &painter)
{
	painter.save();

	int nSize = _vecDataItem.size();	//���������Item��
	int nByte = 16 / _dataCols;				//һ��Item��ռ�õ��ֽ���

	unsigned long long addr = 0;
	int index = 0;
	for (int i = 0; i < nSize; ++i) {
		addr = _firstAddr + i*nByte;	//Item��Ӧ�����ݵ�ַ
		index = i*nByte;				//Item��Ӧ�������±�����

		// ��ȡ����
		_vecDataItem[i].text.swap(getHexData(addr, index));

		if (addr >= _dataAddr1 && addr <= _dataAddr2) {
			painter.setPen(Qt::red);
			painter.setFont(_boldFont);
		
			// ������������ݣ����Ʊ���ɫ
			if (_bClicked) {
				//int width = _pFontMetrics->width(_row.Height);
				_rtClickedArea.setRect(_vecDataItem.at(i).rect.x(), _vecDataItem.at(i).rect.y(), _maxDataWidth, _rowHeight);
				painter.fillRect(_rtClickedArea, QColor(0, 151, 251));
				painter.setPen(Qt::white);
			}
		}
		else {
			//�ָ�Ĭ���������ɫ
			painter.setPen(_penColor);
			painter.setFont(_originalFont);
		}


		//���䶯�������ϱ���ɫɫ
		auto it = _hashAddrTable.find(addr);
		if (it != _hashAddrTable.end()) {
			QRect rt(_vecDataItem.at(i).rect);
			rt.setRect(rt.x(), rt.y(), _pFontMetrics->width(_vecDataItem.at(i).text), rt.height());

			painter.fillRect(rt, QColor(255, 0, 0, it.value().diaphaneity));
		

			QRgb sadf = painter.pen().color().rgb();
			sadf ^= QColor(0, it.value().diaphaneity, 0).rgb();

			painter.setPen(QColor(sadf));
		}

		// ��������
		painter.drawText(_vecDataItem.at(i).rect, Qt::AlignLeft | Qt::AlignVCenter, _vecDataItem.at(i).text);
	}

	painter.restore();
}

/*
* @breif 	��ȡUTF-8 �ַ�����
* @param	byte:1���ֽ�����
* @return 	0:UTF-8 �����ֽڣ�����0:UTF-8 �ַ��ֽ���
*/
uint8_t u8bytes(uint8_t byte)
{
	int bytes = 0;

	if (byte <= 0x7F) { //then ASCII ռ��1���ֽ�
		bytes = 1;
	}
	else if (byte >= 0xC0 && byte <= 0xDF) {  // then ���ֽ�   UTF-8 ռ��2���ֽ�
		bytes = 2;
	}
	else if (byte >= 0xE0 && byte <= 0xEF) {  // then ���ֽ�   UTF-8 ռ��3���ֽ�
		bytes = 3;
	}
	else if (byte >= 0xF0 && byte <= 0xF7) {  // then ���ֽ�   UTF-8 ռ��4���ֽ�
		bytes = 4;
	}
	else if (byte >= 0xF8 && byte <= 0xFB) {  // then ���ֽ�   UTF-8 ռ��5���ֽ�
		bytes = 5;
	}
	else if (byte >= 0xFC && byte <= 0xFD) {  // then ���ֽ�   UTF-8 ռ��6���ֽ�
		bytes = 6;
	}
	else if (byte > 0x7F && byte < 0xC0) {   // then UTF-8   �����ֽ�
		bytes = 0;
	}

	return bytes;
}

void HexEdit::drawTextData(QPainter &painter)
{
	painter.save();
	
	painter.setPen(QColor(0, 128, 54));


	int nSize = _vecTextItem.size();	//�ı������Item��
	int nByte = 16 / _textCols;				//һ��Item��ռ�õ��ֽ���
	//QString text;
	unsigned int len;
	unsigned long long addr = 0;
	int index = 0;

	for (int i = 0; i < nSize; ++i) {
		addr = _firstAddr + i*nByte;	//Item��Ӧ�����ݵ�ַ
		index = i*nByte;				//Item��Ӧ�������±�����
				
		len = getTextData(addr, index, i, _vecTextItem[i].text);

		
		if (len > 1) {
			QRect ttt = _vecTextItem.at(i).rect;
			ttt.setRect(ttt.x(), ttt.y(), _rtTextItemWidth * len, _rowHeight);
			_vecTextItem[i].rect = ttt;
		}
		

		if (addr >= _dataAddr1 && addr <= _dataAddr2) {
			painter.setPen(Qt::red);
			painter.setFont(_boldFont);

			if (_bClicked) {
				painter.fillRect(_vecTextItem.at(i).rect, QColor(0, 151, 251));		
				painter.setPen(Qt::white);
			}
		}
		else {
			painter.setPen(_penColor);
			painter.setFont(_originalFont);
		}


		//���䶯�������ϱ���ɫɫ
		auto it = _hashAddrTable.find(addr);
		if (it != _hashAddrTable.end()) {
			QRect rt(_vecTextItem.at(i).rect);
			rt.setRect(rt.x(), rt.y(), _pFontMetrics->width(_vecTextItem[i].text), rt.height());

			painter.fillRect(rt, QColor(255, 255, 0, it.value().diaphaneity));
			painter.setPen(Qt::red);
		}
	
		painter.drawText(_vecTextItem.at(i).rect, Qt::AlignLeft | Qt::AlignVCenter, _vecTextItem[i].text);

		i = --i + len;
	}

	painter.restore();
}




QString HexEdit::getHexData(unsigned long long addr, long long index)
{
	bool b = true;	//�����Ƿ���Է���


	if (addr < _mi.endAddr) {
		_penColor = _mi.isBase ? QColor("#008036") : QColor("#000000");
		b = _mi.state != 0x1000 ? false : true;
	}
	else {
		//���ڵ���_mi.endAddr˵���õ�ַ���ڵڶ����ڴ�
		_penColor = _mi.color2;
		b = _mi.state2 != 0x1000 ? false : true;
	}

	switch (_dataType)
	{
	case e1ByteHex:
		return b? QString("%1").arg(_pData[index], 2, 16, QChar('0')).toUpper() : QStringLiteral("??");
	case e2ByteHex:
		return b ? QString("%1").arg(*(unsigned short*)&_pData[index], 4, 16, QChar('0')).toUpper() : QStringLiteral("????");
	case e4ByteHex:
		return b ? QString("%1").arg(*(unsigned int*)&_pData[index], 8, 16, QChar('0')).toUpper() : QStringLiteral("????????");
	case e8ByteHex:
		return b ? QString("%1").arg(*(unsigned long long*)&_pData[index], 16, 16, QChar('0')).toUpper() : QStringLiteral("????????????????");
	case e1ByteDec:
		return b ? _bSymbol ? QString::number(*(char*)&_pData[index]) : QString::number(_pData[index]) : QStringLiteral("??");
	case e2ByteDec:
		return b ? _bSymbol ? QString::number(*(short*)&_pData[index]) : QString::number(*(unsigned short*)&_pData[index]) : QStringLiteral("????");
	case e4ByteDec:
		return b ? _bSymbol ? QString::number(*(int*)&_pData[index]) : QString::number(*(unsigned int*)&_pData[index]) : QStringLiteral("????????");
	case e8ByteDec:
		return b ? _bSymbol ? QString::number(*(long long*)&_pData[index]) : QString::number(*(unsigned long long*)&_pData[index]) : QStringLiteral("????????????????");
	case e4ByteFloat: {
		QFontMetrics fontWidth(this->font());
		QString str = QString::number(*(float*)&_pData[index], 'f', 2);
		int width = fontWidth.width(str);
		int maxWidth = fontWidth.width(QStringLiteral("2147483648"));
		str = fontWidth.elidedText(str, Qt::ElideRight, maxWidth);
		return b ? str : QStringLiteral("????????");
	}
	case e8ByteDouble: {
		QFontMetrics fontWidth(this->font());
		QString str = QString::number(*(double*)&_pData[index], 'd', 2);
		int width = fontWidth.width(str);
		int maxWidth = fontWidth.width(QStringLiteral("-9223372036854775808"));
		str = fontWidth.elidedText(str, Qt::ElideRight, maxWidth);
		return b ? str : QStringLiteral("????????????????");
	}
	default:
		break;
	}
	return QString();
}

unsigned int HexEdit::getTextData(unsigned long long addr, long long index, int &itemIndex, QString &text)
{
	if (addr < _mi.endAddr) {
		_penColor = _mi.isBase ? QColor("#008036") : QColor("#000000");
		if (_mi.state != 0x1000) {
			text = QStringLiteral("?");
			return 1;
		}
	}
	else {
		_penColor = _mi.color2;
		if (_mi.state2 != 0x1000) {
			text = QStringLiteral("?");
			return 1;
		}
	}

	int len = 1;
	switch (_textType)
	{
	case eASCII: {
		unsigned char ch = _pData[index];
		if (iscntrl(ch)) {
			ch = '.';	//�ǿ��Ʒ�����.��ʾ
		}
		else if (!isprint(ch)) {
			ch = ' ';	//���ɴ�ӡ���ַ��� ��ʾ
		}
		text = QChar(ch);
		break;
	}	
	case eUTF8: {
		len = u8bytes(_pData[index]);

		char u8[7]{};
		memcpy(u8, &_pData[index], len);

		if (len > 1 && prc->isUTF8����(u8)) {
			text = u8;
		}
		else {
			unsigned char ch = _pData[index];
			if (iscntrl(ch)) {
				ch = '.';
			}
			else if (!isprint(ch)) {
				ch = ' ';
			}
			text = QChar(ch);
			len = 1;
		}

		break;
	}
	case eUTF16: {
		wchar_t wch = *(unsigned short*)&_pData[index];
		wchar_t wch2 = *(unsigned short*)&_pData[index + 2];

		if ((wch >= 0x0000 && wch <= 0xD7FF) || (wch >= 0xE000 && wch <= 0xFFFF)) {
			if (iswcntrl(wch)) {
				wch = L'.';
			}
			else if (!iswprint(wch)) {
				wch = L' ';
			}
			text = QChar(wch);
			len = 2;
		}
		else if ((wch >= 0xD800 && wch <= 0xDBFF) || (wch2 >= 0xDC00 && wch2 <= 0xDFFF)) {
			wch = (wch & 0x03FF) << 10;
			wch2 = (wch2 & 0x03FF);
			auto wch3 = (wch | wch2) + 0x10000;

			wchar_t aaa[2]{};
			memcpy(aaa, &wch3, 4);
			text = QString::fromWCharArray(aaa);
			len = 4;
		}	
		break;
	}
	default:
		break;
	}
	return len;
}


const QRect* pRt = nullptr;
void HexEdit::wheelEvent(QWheelEvent * event)
{
	if (event->orientation() == Qt::Vertical){
		if (event->delta() < 0) // Scroll Down
			_firstAddr += 16 * 3;
		else if (event->delta() > 0) // Scroll Up
			_firstAddr -= 16 * 3;

		changeAddr(_firstAddr);

		if (_bPress) {
			pRt = nullptr;
			setEndPos(event->pos());
		}
		
		viewport()->update();
		return;
	}

	QAbstractScrollArea::wheelEvent(event);
}

//const QRect* pBeginRt = nullptr;
void HexEdit::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton) {
		//���ñ�־
		_bPress = true;
		_bClicked = false;
		//
		setBeginPos(event->pos());
	}	
}

void HexEdit::mouseReleaseEvent(QMouseEvent * event)
{
	_bPress = false;

	if (event->button() == Qt::RightButton){
		// ���º͵���ָ������ݶ�Ӧ��ʵ�ʵ�ַ��һ��������Ϊ���
		if (_dataAddr1 == _dataAddr2) {
			setBeginPos(event->pos());
			_bClicked = true;
		}
		_pMenu->exec(QCursor::pos());
	}
	if (event->button() == Qt::LeftButton) {
		// ���º͵���ָ������ݶ�Ӧ��ʵ�ʵ�ַ��һ��������Ϊ���
		if (_dataAddr1 == _dataAddr2) {
			_bClicked = true;
			viewport()->update();
		}
	}
}


void HexEdit::mouseDoubleClickEvent(QMouseEvent *event) //��갴���¼�
{
	if (event->buttons() & Qt::LeftButton){
		if (_rtDataArea.contains(event->pos())) {
			QVector<HexEditItem>* pVec = &_vecDataItem;
			int n = 16 / _dataCols;
			for (int i = 0; i < pVec->size(); ++i) {
				if (pVec->at(i).rect.contains(event->pos())) { 
					_dataIndex1 = _dataIndex2 = i*n;
					_dataAddr1 = _firstAddr + i*n;
					_dataAddr2 = _dataAddr1 + n - 1;

					//�жϵ���ĵ�ַ�Ƿ���Զ�ȡ
					if (_dataAddr1 < _mi.endAddr) {
						if (_mi.state != 0x1000) {
							return;
						}
					}
					else {
						if (_mi.state2 != 0x1000) {
							return;
						}
					}

					//��ȡ����׼���޸�
					memset(&_info, 0, sizeof(DataInfo));
					memcpy(_info.data, &_pData[_dataIndex1], 128);
					_info.pid = _pid;
					_info.beginAddr = _dataAddr1;
					_info.endAddr = _dataAddr2;
					_info.bSymbol = _bSymbol;
					_info.dataType = _dataType;

					//emit signal_DoubleData(_info, _dataType);

					_pEditorWidgets->showType(_info, 0);

					break;
				}
			}
		}
		else if (_AllocationBaseAddrItem.rect.contains(event->pos())) {
			_firstAddr = _mi.allocationBase;
			changeAddr(_firstAddr);
			viewport()->update();
		}
		else if (_BaseAddrItem.rect.contains(event->pos())) {
			_firstAddr = _mi.beginAddr;
			changeAddr(_firstAddr);
			viewport()->update();
		}
		else if (_AddrSizeItem.rect.contains(event->pos())) { 
			_firstAddr = _mi.endAddr;
			changeAddr(_firstAddr);
			viewport()->update();
		}
	}
}

void HexEdit::mouseMoveEvent(QMouseEvent * event)
{
	if (_bPress) {
		setEndPos(event->pos());
	}
}

unsigned long long g_dataAddr = 0;
unsigned long long g_headAddr = 0;
unsigned int g_dataIndex = 0;
unsigned int g_itemIndex = 0;
void HexEdit::setBeginPos(QPoint &pos)
{
	QVector<HexEditItem>* pVec;
	int nCols = 0;
	if (_rtDataArea.contains(pos)) {
		pVec = &_vecDataItem;
		nCols = _dataCols;
		_releaseArea = 1;
	}
	else if (_rtTextArea.contains(pos)) {
		pVec = &_vecTextItem;
		nCols = _textCols;
		_releaseArea = 2;
	}
	else {
		return;
	}

	int n = 16 / nCols;
	for (int i = 0; i < pVec->size(); ++i) {
		if (pVec->at(i).rect.contains(pos)) { 

			g_dataIndex = _dataIndex1 = _dataIndex2 = i*n;						//���ݻ���������
			g_dataAddr = _dataAddr1 = _dataAddr2 = _firstAddr + _dataIndex1;		//���ݵ�ʵ�ʵ�ַ
			g_headAddr = _headAddr1 = _headAddr2 = _firstAddr + (int)((_dataAddr1 - _firstAddr) / 16) * 16;	//ÿ�еĵ�һ�����ݶ�Ӧ��ʵ�ʵ�ַ
			g_itemIndex = _itemIndex1 = _itemIndex2 = i;
			_colIndex = i % nCols * n;	//Item���ڵ��к�

			_pLastMouseItem = &pVec->at(i);	//���һ�����ָ���Item
			viewport()->update();

			//�����ȡ���ݣ�����״̬��
			memset(&_info, 0, sizeof(DataInfo));
			memcpy(_info.data, &_pData[_dataIndex1], 128);
			_info.pid = _pid;
			_info.beginAddr = _dataAddr1;
			_info.endAddr = _dataAddr2;
			_info.byteNum = _info.endAddr - _info.beginAddr + 1;
			_info.bSymbol = _bSymbol;
			_info.dataType = _dataType;
			emit signal_updateStatusBar(_info);

			break;
		}
	}
}
void HexEdit::setEndPos(QPoint &pos)
{
	QVector<HexEditItem>* pVec;
	int nCols = 0;
	if (_pLastMouseItem && _pLastMouseItem->rect.contains(pos)) { 
		return;
	}
	else if (_rtDataArea.contains(pos)) {
		pVec = &_vecDataItem;
		nCols = _dataCols;
		_releaseArea = 1;
	}
	else if (_rtTextArea.contains(pos)) {
		pVec = &_vecTextItem;
		nCols = _textCols;
		_releaseArea = 2;
	}
	else {
		//_releaseArea = 0;
		return;
	}
	
	int n = 16 / nCols;
	int index = 0;
	for (int i = 0; i < pVec->size(); ++i) {
		if (pVec->at(i).rect.contains(pos)) {

			_dataIndex2 = i*n;														//���ݻ���������
			_dataAddr2 = _firstAddr + _dataIndex2;									//���ݵ�ʵ�ʵ�ַ
			_headAddr2 = _firstAddr + (int)((_dataAddr2 - _firstAddr) / 16) * 16;	//ÿ�еĵ�һ�����ݶ�Ӧ��ʵ�ʵ�ַ
			_itemIndex2 = i;

			_pLastMouseItem = &pVec->at(i);		//���ָ���Item

			break;
		}
	}
	
	//�Ƿ���Ҫ������ַ
	_dataAddr1 = g_dataAddr;
	_headAddr1 = g_headAddr;
	_dataIndex1 = g_dataIndex;
	_itemIndex1 = g_itemIndex;
	if (_dataAddr2 < _dataAddr1) {
		std::swap(_dataAddr1, _dataAddr2);
		std::swap(_headAddr1, _headAddr2);
		std::swap(_dataIndex1, _dataIndex2);
		std::swap(_itemIndex1, _itemIndex2);
	}
	


	//�����ȡ���ݣ�����״̬��
	memset(&_info, 0, sizeof(DataInfo));
	memcpy(_info.data, &_pData[index], 128);
	_info.pid = _pid;
	_info.beginAddr = _dataAddr1;
	_info.endAddr = _dataAddr2;
	_info.byteNum = _info.endAddr - _info.beginAddr + 1;
	_info.bSymbol = _bSymbol;
	_info.dataType = _dataType;

	//����״̬��
	emit signal_updateStatusBar(_info);

	viewport()->update();
}

void HexEdit::slot_dataType()
{
	QObject *object = QObject::sender();
	if (object) {
		for (auto t : _vecAction_DataType) {
			t->setChecked(false);
		}

		QAction *pAction = qobject_cast<QAction *>(object);
		pAction->setChecked(true);

		QString text = pAction->text();
		if (text == QStringLiteral("�ֽڣ�ʮ�����ƣ�")) {
			_dataType = e1ByteHex;		
		}
		else if (text == QStringLiteral("�ֽڣ�ʮ���ƣ�")) {
			_dataType = e1ByteDec;		
		}
		else if (text == QStringLiteral("2�ֽڣ�ʮ�����ƣ�")) {
			_dataType = e2ByteHex;
		}
		else if (text == QStringLiteral("2�ֽڣ�ʮ���ƣ�")) {
			_dataType = e2ByteDec;	
		}
		else if (text == QStringLiteral("4�ֽڣ�ʮ�����ƣ�")) {
			_dataType = e4ByteHex;
		}
		else if (text == QStringLiteral("4�ֽڣ�ʮ���ƣ�")) {
			_dataType = e4ByteDec;	
		}
		else if (text == QStringLiteral("8�ֽڣ�ʮ�����ƣ�")) {
			_dataType = e8ByteHex;
		}
		else if (text == QStringLiteral("8�ֽڣ�ʮ���ƣ�")) {
			_dataType = e8ByteDec;	
		}
		else if (text == QStringLiteral("��������")) {
			_dataType = e4ByteFloat;
		}
		else if (text == QStringLiteral("˫������")) {
			_dataType = e8ByteDouble;
		}
	}

	_updateViewItem();
}

void HexEdit::slot_textType()
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
	}

	_updateViewItem();
}

void HexEdit::slot_protectType()
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

		HANDLE hPro = prc->openProsess(_pid);

		// �ڴ���Ϣ
		MEMORY_BASIC_INFORMATION mbi{};
		memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
		::VirtualQueryEx(hPro, (LPCVOID)_dataAddr1, &mbi, sizeof(mbi));

		DWORD lpflOldProtect;
		QAction *pAction = qobject_cast<QAction *>(object);
		QString text = pAction->text();
		if (text == QStringLiteral("ִ��/��д")) {	
			VirtualProtectEx(hPro, (LPVOID)mbi.BaseAddress, sizeof(mbi.RegionSize), PAGE_EXECUTE_READWRITE, &lpflOldProtect);
		}
		else if (text == QStringLiteral("ִ��/��")) {
			VirtualProtectEx(hPro, (LPVOID)mbi.BaseAddress, sizeof(mbi.RegionSize), PAGE_EXECUTE_READ, &lpflOldProtect);	
		}
		else if (text == QStringLiteral("��д")) {
			VirtualProtectEx(hPro, (LPVOID)mbi.BaseAddress, sizeof(mbi.RegionSize), PAGE_READWRITE, &lpflOldProtect);
		}
		else if (text == QStringLiteral("ֻ��")) {
			VirtualProtectEx(hPro, (LPVOID)mbi.BaseAddress, sizeof(mbi.RegionSize), PAGE_READONLY, &lpflOldProtect);	
		}
		_mi.strProtect = text;
		viewport()->update();
	}
}

void HexEdit::slot_divisionType()
{
	QObject *object = QObject::sender();
	if (object) {
		for (auto t : _vecAction_divisionType) {
			t->setChecked(false);
		}

		QAction *pAction = qobject_cast<QAction *>(object);
		pAction->setChecked(true);

		QString text = pAction->text();
		if (text == QStringLiteral("2 �ֽ�")) {
			_divisionByte = 2;
		}
		else if (text == QStringLiteral("4 �ֽ�")) {
			_divisionByte = 4;
		}
		else if (text == QStringLiteral("8 �ֽ�")) {
			_divisionByte = 8;
		}
		viewport()->update();
	}
}

//˫���޸�����
void HexEdit::slot_changeData(DataInfo info, int size)
{
	//HANDLE hProc = ::OpenProcess(PROCESS_ALL_ACCESS, false, _pid);
	//bool b = WriteProcessMemory(hProc, (LPVOID)info.beginAddr, info.data, size, NULL);
	//CloseHandle(hProc);

	_pUndoStack->push(new WriteCommand(_pid, info.beginAddr, &_pData[_dataIndex1], info.data, size));
}


void HexEdit::slot_symbol()
{
	_bSymbol = !_bSymbol;
	_updateViewItem();
}


void HexEdit::slot_actionTriggered(int action)
{
	switch (action)
	{
	case QAbstractSlider::SliderMove:
		_bSliderMove = true;
		break;
	case QAbstractSlider::SliderSingleStepAdd:
		_firstAddr += 0x10;
		changeAddr(_firstAddr);
		break;
	case QAbstractSlider::SliderSingleStepSub:
		_firstAddr -= 0x10;
		changeAddr(_firstAddr);
		break;
	case QAbstractSlider::SliderPageStepAdd:
		_firstAddr += 0x30;
		changeAddr(_firstAddr);
		break;
	case QAbstractSlider::SliderPageStepSub:
		_firstAddr -= 0x30;
		changeAddr(_firstAddr);
		break;
	default:
		_bSliderMove = false;
		break;
	}
}

void HexEdit::slot_sliderPressed()
{
	_nTimerID = this->startTimer(100);
}

void HexEdit::slot_sliderReleased()
{
	killTimer(_nTimerID);
	//���û���λ�õ��м�
	verticalScrollBar()->setValue(_middleValue);
}

void HexEdit::slot_toAddr()
{
	_pEditorWidgets->showType(_info, 1);
}
void HexEdit::slot_changeTime()
{
	_pEditorWidgets->showType(_info, 2);
}

void HexEdit::slot_gotoAddr(unsigned long long addr)
{
	if (_bClicked) {
		_addrStack.push(_headAddr1);
	}
	else {
		_addrStack.push(_firstAddr);
	}
	

	_firstAddr = addr;
	changeAddr(_firstAddr);

	if (_bClicked) {
		_colIndex = 0;
		_headAddr1 = _headAddr2 = _firstAddr;
	}

	viewport()->update();
}

void HexEdit::slot_setTime(int ms)
{
	_ms = ms;
}


void HexEdit::slot_copy()
{
	QString strData;
	if (_releaseArea == 1) {
		for (int i = _itemIndex1; i <= _itemIndex2; ++i) {
			strData += _vecDataItem.at(i).text + " ";
		}
	}
	else if (_releaseArea == 2) {
		int size = _dataIndex2 - _dataIndex1 + 1;
		char* text = new char[size + 1]();
		memcpy(text, &_pData[_dataIndex1], size);

		bool isStr = true;
		for (int i = 0; i < size; ++i) {
			int len = u8bytes(text[i]);
			if (len == 0) {
				isStr = false;
				break;
			}
			else if (len == 1) {
				if (!isprint(text[i])) {
					isStr = false;
					break;
				}
				else {
					strData += text[i];
				}	
			}
			else if (len > 1) {
				char* sss = new char[len + 1]();
				memcpy(sss, &text[i], len);
				if (prc->isUTF8����(sss)) {
					strData += text;
					delete[] sss;
				}
				else {
					isStr = false;
					delete[] sss;
					break;
				}
				i = --i + len;
			}
		}

		if (!isStr) {
			strData.clear();
			for (int i = _dataIndex1; i <= _dataIndex2; ++i) {
				strData += QString("%1 ").arg(_pData[i], 2, 16, QChar('0')).toUpper();
			}
		}
		
		delete[] text;
	}

	QClipboard *board = QApplication::clipboard();
	board->setText(strData);
}
void HexEdit::slot_paste()
{
	_paste();
}

void HexEdit::_paste()
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
		rx.setPattern(QString("^[A-Fa-f0-9]+$"));
		bbb = rx.exactMatch(str);
		if (bbb) {
			//PLOGD << "��16�����������ֽ����飿";
			int len = str.length();
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
			rx.setPattern(QString("^-?(\\d+(.{1}\\d+))$"));
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
						if (maxLen % 2 != 0) {	//���Ȳ���2�ı�����Ϊ�ַ������� 1BC = 3
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

	_pUndoStack->push(new WriteCommand(_pid, _dataAddr1, &_pData[_dataIndex1], buff, size));

	delete[] buff;
}

void HexEdit::slot_undo()
{
	_pUndoStack->undo();
}
void HexEdit::slot_redo()
{
	_pUndoStack->redo();
}

void HexEdit::slot_backspaceAddr()
{
	if (_addrStack.size() > 0) {
		_firstAddr = _addrStack.pop();
		//_firstAddr = _headAddr1;
	}
	else if (qAbs((long long)(_firstAddr - _headAddr1)) > 400) {
		_firstAddr = _headAddr1;
	}
	else {
		return;
	}

	_bClicked = false;
	changeAddr(_firstAddr);

	viewport()->update();
}

void HexEdit::updeteView()
{
	viewport()->update();
}