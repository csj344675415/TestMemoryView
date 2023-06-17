#include "MemoryView.h"
#include <QResizeEvent>
#include <QScrollBar>
#include <QPainter>
#include <QPaintEvent>
#include <QMenu>


MemoryView::MemoryView(QWidget *parent)
	: QAbstractScrollArea(parent)
{
	setWindowIcon(QIcon(":/MyWidgets/MemoryWidgets/Resources/monitor system.png"));	//设置窗口标题栏图标
	setWindowTitle(QStringLiteral("内存视图"));										//设置窗口标题
#if _WIN64
	this->resize(700, 445);
#else
	this->resize(620, 445);
#endif

	setMouseTracking(true);
	setFont(QFont("Courier New", 10));
	_pMetrics = new QFontMetrics(this->font());
	_rowHeight = _pMetrics->height();

	_buffer = new DataBuffer(this);
	connect(_buffer, &DataBuffer::signal_dataChange, this, [this] {viewport()->update(); });
	connect(_buffer, &DataBuffer::signal_update, this, [this] {viewport()->update(); });

	_pEditor = new EditorWidgets;
	connect(_pEditor, &EditorWidgets::signal_changeData, this, &MemoryView::slot_changeData);
	connect(_pEditor, &EditorWidgets::signal_changeTime, this, &MemoryView::slot_changeTime);
	connect(_pEditor, &EditorWidgets::signal_changeAddr, this, &MemoryView::slot_changeAddr2);

	_pUndoStack = new QUndoStack(this);

	init();
	initMenu();
	initShortcut();
}

MemoryView::~MemoryView()
{
	delete _pEditor;
	delete _buffer;
	delete _pUndoStack;
	delete _pMetrics;
}

void MemoryView::resizeEvent(QResizeEvent *event)
{
	int rows = event->size().height() / _rowHeight;
	if (rows != _maxRowCount) {
		setItem();
	}
}

void MemoryView::closeEvent(QCloseEvent *event)
{
	updateStop();
	event->accept();
}

void MemoryView::updateStop()
{
	_buffer->updateStop();
}

void MemoryView::show(unsigned int pid, unsigned long long addr)
{
	setData(pid, addr);
	QWidget::show();
}

void MemoryView::init()
{
	// 初始化滚动条
	verticalScrollBar()->setRange(0, 0x7FFFFFFF);
	verticalScrollBar()->setPageStep(0x10 * 3);	//按下滑块与箭头按钮之间的区域时移动的步长
	verticalScrollBar()->setSingleStep(0x10);	//按下箭头按钮时移动的步长
	_middleValue = verticalScrollBar()->maximum() / 2;
	verticalScrollBar()->setValue(_middleValue);
	connect(verticalScrollBar(), SIGNAL(actionTriggered(int)), this, SLOT(slot_actionTriggered(int)));	//滚动条动作类型
	connect(verticalScrollBar(), SIGNAL(sliderPressed()), this, SLOT(slot_sliderPressed()));	//按下滑块
	connect(verticalScrollBar(), SIGNAL(sliderReleased()), this, SLOT(slot_sliderReleased()));	//释放滑块
}



void MemoryView::setData(unsigned int pid, unsigned long long addr)
{
	_pid = pid;
	//addr = 0x0100C9F0;
	_mi = _buffer->setData(pid, addr);
	_firstAddr = _mi.firstAddr;
	viewport()->update();
}

void MemoryView::setItem()
{
	_rowHeight = _pMetrics->height();
	_maxRowCount = this->rect().height() / _rowHeight;
	_maxByteCount = _maxRowCount * 16;
	_addrStringWidth = getStringWidth(eAddr_Width);	
	_dataStringWidth = getStringWidth(eData_Width);
	_textStringWidth = getStringWidth(eText_Width);
	_addrItemWidth = _addrStringWidth + 8;
	_dataItemWidth = getDataItemWidth();
	_textItemWidth = _textStringWidth;
	//_dataColumnCount = 
	//_textColumnCount = 

	// 背景区域
	_rtMemInfoArea.setRect(this->rect().x(), this->rect().y(), 1920, _rowHeight + 4);
	_rtOffsetArea.setRect(this->rect().x(), _rtMemInfoArea.bottom(), 1920, _rowHeight);
	_rtAddrArea.setRect(this->rect().x(), _rtOffsetArea.bottom(), _addrItemWidth, this->rect().height());
	_rtDataArea.setRect(_rtAddrArea.right() + 4, _rtOffsetArea.bottom(), _dataItemWidth * _dataColumnCount, this->rect().height());
	_rtTextArea.setRect(_rtDataArea.right() + 4, _rtOffsetArea.bottom(), _textItemWidth * _textColumnCount, this->rect().height());

	_maxWidgetWidth = _rtAddrArea.width() + _rtDataArea.width() + _rtTextArea.width();

	// Item
	_vecAddrItem.clear();
	_vecDataItem.clear();
	_vecTextItem.clear();
	HexEditItem t1{}, t2{}, t3{};
	for (int i = 0; i < _maxRowCount; ++i) {
		t1.rect.setRect(_rtAddrArea.x(), (_rtAddrArea.y() + 2) + i * _rowHeight, _rtAddrArea.width(), _rowHeight);
		_vecAddrItem.append(t1);

		for (int j = 0; j < _dataColumnCount; ++j) {
			t2.rect.setRect(_rtDataArea.x() + j * _dataItemWidth, t1.rect.y(), _dataItemWidth, _rowHeight);
			_vecDataItem.append(t2);
		}

		for (int k = 0; k < _textColumnCount; ++k) {
			t3.rect.setRect(_rtTextArea.x() + k * _textItemWidth, t1.rect.y(), _textItemWidth, _rowHeight);
			_vecTextItem.append(t3);
		}
	}
}

int MemoryView::getStringWidth(int id)
{
	int w = 0;
	if (id == eAddr_Width) {
#if _WIN64
		w = _bDrawRelativeAddress ? _pMetrics->width(QStringLiteral("-FFFFFFFFFFFFFFFF")) : _pMetrics->width(QStringLiteral("FFFFFFFFFFFFFFFF"));
#else
		w = _bDrawRelativeAddress ? _pMetrics->width(QStringLiteral("-FFFFFFFF")) : _pMetrics->width(QStringLiteral("FFFFFFFF"));
#endif
	}
	else if (id == eData_Width) {
		switch (_dataType)
		{
		case e1ByteHex:_dataColumnCount = 16; w = _pMetrics->width(QStringLiteral("FF")); break;	//包围一个数据字符串的宽度
		case e1ByteDec:_dataColumnCount = 16; w = _pMetrics->width(QStringLiteral("-127")); break;
		case e2ByteHex:_dataColumnCount = 8; w = _pMetrics->width(QStringLiteral("FFFF")); break;
		case e2ByteDec:_dataColumnCount = 8; w = _pMetrics->width(QStringLiteral("-32768")); break;
		case e4ByteHex:_dataColumnCount = 4; w = _pMetrics->width(QStringLiteral("FFFFFFFF")); break;
		case e4ByteDec:_dataColumnCount = 4; w = _pMetrics->width(QStringLiteral("-2147483648")); break;
		case e8ByteHex:_dataColumnCount = 2; w = _pMetrics->width(QStringLiteral("FFFFFFFFFFFFFFFF")); break;
		case e8ByteDec:_dataColumnCount = 2; w = _pMetrics->width(QStringLiteral("-9223372036854775808")); break;
		case e4ByteFloat:_dataColumnCount = 4; w = _pMetrics->width(QStringLiteral("-2147483648")); break; break;
		case e8ByteDouble:_dataColumnCount = 2; w = _pMetrics->width(QStringLiteral("-9223372036854775808")); break;
		}

#if _WIN64
		_dataType == e1ByteDec ? emit signal_resize(849) : emit signal_resize(700);
#else
		_dataType == e1ByteDec ? emit signal_resize(786) : emit signal_resize(620);
#endif
	}
	else if (id == eText_Width) {
		switch (_textType)
		{
		case eASCII:_textColumnCount = 16; w = _pMetrics->width(QStringLiteral("F"));	break;
		case eUTF8:_textColumnCount = 16; w = _pMetrics->width(QStringLiteral("F")); break;
		case eUTF16:_textColumnCount = 16; w = _pMetrics->width(QStringLiteral("F")); break;
		}
	}

	return w;
}

int MemoryView::getDataItemWidth()
{
	int w = _dataStringWidth;
	int w2 = _pMetrics->width(QStringLiteral("F"));
	if ((w + 8) * _dataColumnCount <= (w2 * 2 + 8) * 16) {
		w = ((w2 * 2 + 8) * 16) / _dataColumnCount;
	}
	else {
		if (_bSymbol) {
			w = w + 8 - 2;	
		}
		else {
			w = w + 8 - 6;	
		}
	}
	return w;
}

void MemoryView::slot_actionTriggered(int action)
{
	switch (action)
	{
	case QAbstractSlider::SliderMove:
		_bSliderMove = true;
		break;
	case QAbstractSlider::SliderSingleStepAdd:
		_firstAddr += 0x10;
		_mi = _buffer->changeAddr(_firstAddr);
		break;
	case QAbstractSlider::SliderSingleStepSub:
		_firstAddr -= 0x10;
		_mi = _buffer->changeAddr(_firstAddr);
		break;
	case QAbstractSlider::SliderPageStepAdd:
		_firstAddr += 0x30;
		_mi = _buffer->changeAddr(_firstAddr);
		break;
	case QAbstractSlider::SliderPageStepSub:
		_firstAddr -= 0x30;
		_mi = _buffer->changeAddr(_firstAddr);
		break;
	default:
		_bSliderMove = false;
		break;
	}
}

void MemoryView::slot_sliderPressed()
{
	_nTimerID = this->startTimer(300);
}

void MemoryView::slot_sliderReleased()
{
	killTimer(_nTimerID);							//停止定时器
	verticalScrollBar()->setValue(_middleValue);	//重置滑块位置到中间
}

void MemoryView::timerEvent(QTimerEvent *event) 
{
	if (event->timerId() == _nTimerID) {
		int value = verticalScrollBar()->value();

		float f = qAbs(value - _middleValue) / (float)_middleValue;
		int n = f < 0.5 ? f * 25 : f * 50;

		if (value > _middleValue) {
			_firstAddr += 0x10 * n;
		}
		else if (value < _middleValue) {
			_firstAddr -= 0x10 * n;
		}
		_mi = _buffer->changeAddr(_firstAddr);
		viewport()->update();
	}
}

void MemoryView::wheelEvent(QWheelEvent * event)
{
	if (event->orientation() == Qt::Vertical) {
		if (event->delta() < 0) // Scroll Down
			_firstAddr += 16 * 3;
		else if (event->delta() > 0) // Scroll Up
			_firstAddr -= 16 * 3;

		_mi = _buffer->changeAddr(_firstAddr);

		if (_bPress) {
			_pLastItem = nullptr;
			auto pos = event->pos();
			setEndPos(pos);
		}

		viewport()->update();
		return;
	}

	QAbstractScrollArea::wheelEvent(event);
}

void MemoryView::paintEvent(QPaintEvent *event)
{
	QPainter painter(viewport());

	// 绘制背景矩形
	painter.fillRect(_rtMemInfoArea, this->palette().alternateBase().color());
	painter.fillRect(_rtOffsetArea, this->palette().alternateBase().color());
	painter.fillRect(_rtAddrArea, this->palette().alternateBase().color());

	// 绘制区域分割线
	painter.setPen(Qt::lightGray);
	painter.drawLine(_rtAddrArea.right(), _rtOffsetArea.bottom(), _rtAddrArea.right(), event->rect().bottom());	//竖线
	painter.setPen(Qt::black);
	painter.drawLine(0, _rtOffsetArea.bottom(), event->rect().right(), _rtOffsetArea.bottom());	//横线

																								// 
	painter.setPen(Qt::black);
	_strAddrHeadItem.rect = _vecAddrItem.at(0).rect;
	_strAddrHeadItem.rect.setRect(_strAddrHeadItem.rect.x() + 4, _rtOffsetArea.y(), _strAddrHeadItem.rect.width(), _strAddrHeadItem.rect.height());
	painter.drawText(_strAddrHeadItem.rect, Qt::AlignLeft | Qt::AlignVCenter, QStringLiteral("地址"));

	// 填充Item
	drawMemoryInfo(painter);	//绘制内存信息
	drawAddress(painter);		//绘制地址
	drawData(painter);			//绘制数据
	drawTextData(painter);		//绘制文本数据
	drawDataHead(painter);		//绘制数据区域表头
	drawTextDataHead(painter);	//绘制文本区域表头
	drawDivisionLine(painter);	//绘制分割符

}

void MemoryView::drawMemoryInfo(QPainter &painter)
{
	painter.save();

	// 绘制内存基本信息
	QString str = QString(QStringLiteral("保护：%1")).arg(_mi.strProtect);
	int w = _pMetrics->width(str) + 16;
	_strProtectItem.rect.setRect(this->rect().x() + 4, this->rect().y() + 2, w, _rowHeight);
	painter.setPen(Qt::black);
	painter.drawText(_strProtectItem.rect, Qt::AlignLeft | Qt::AlignVCenter, str);

	str = QStringLiteral("AllocationBase=") + QString("%1").arg(_mi.allocationBase, 0, 16, QChar('0')).toUpper();
	w = _pMetrics->width(str) + 8;
	_strAllocationBaseAddrItem.rect.setRect(_strProtectItem.rect.right(), _strProtectItem.rect.y(), w, _rowHeight);
	painter.drawText(_strAllocationBaseAddrItem.rect, Qt::AlignLeft | Qt::AlignVCenter, str);

	str = QString(QStringLiteral("基址=%1")).arg(_mi.beginAddr, 0, 16, QChar('0')).toUpper();
	w = _pMetrics->width(str) + 8;
	_strBaseAddrItem.rect.setRect(_strAllocationBaseAddrItem.rect.right(), _strAllocationBaseAddrItem.rect.y(), w, _rowHeight);
	painter.drawText(_strBaseAddrItem.rect, Qt::AlignLeft | Qt::AlignVCenter, str);

	str = QString(QStringLiteral("大小=%1")).arg(_mi.size, 0, 16, QChar('0')).toUpper();
	w = _pMetrics->width(str) + 8;
	_strAddrSizeItem.rect.setRect(_strBaseAddrItem.rect.right(), _strBaseAddrItem.rect.y(), w, _rowHeight);
	painter.drawText(_strAddrSizeItem.rect, Qt::AlignLeft | Qt::AlignVCenter, str);

	str = QString(QStringLiteral("%1")).arg(_mi.moduleName);
	w = _pMetrics->width(str) + 8;
	_strModuleNameItem.rect.setRect(_strAddrSizeItem.rect.right(), _strAddrSizeItem.rect.y(), w, _rowHeight);
	painter.drawText(_strModuleNameItem.rect, Qt::AlignLeft | Qt::AlignVCenter, str);

	painter.restore();
}

void MemoryView::drawAddress(QPainter &painter)
{
	painter.save();

	painter.setPen(Qt::black);

	for (int i = 0; i < _maxRowCount; ++i) {
		// 取得地址数据
		unsigned long long addr = _firstAddr + i * 0x10;

		if (_bDrawRelativeAddress) {
			if ((addr - _relativeAddress) == 0) {
				QString str = "===> ";
				_vecAddrItem[i].text.swap(str);
			}
			else {
				long long nnn = (long long)(addr - _relativeAddress);
				if (nnn > 0) {
					QString str = QString("+%1 ").arg(QString::number(nnn, 16)).toUpper();
					_vecAddrItem[i].text.swap(str);
				}
				else {
					QString str = QString("-%1 ").arg(QString::number(qAbs(nnn), 16)).toUpper();
					_vecAddrItem[i].text.swap(str);
				}
			}
		}
		else
		{
#if _WIN64
			QString str = QString("%1").arg(addr, 16, 16, QChar('0')).toUpper();
			_vecAddrItem[i].text.swap(str);
#else
			QString str = QString("%1").arg(addr, 8, 16, QChar('0')).toUpper();
			_vecAddrItem[i].text.swap(str);
#endif
			//if (_bClicked && addr == _headAddr1) {
			//	// 绘制点击行的背景色
			//	QRect rtrt = _vecAddrItem.at(i).rect;
			//	rtrt.setRect(rtrt.right() + 1, rtrt.y(), 1920, rtrt.height());
			//	painter.fillRect(rtrt, QColor(245, 245, 245));
			//}

			//if (addr >= _headAddr1 && addr <= _headAddr2) {
			//	// 高亮地址
			//	painter.setPen(QColor("#3399FF"));
			//}
			//else {
			//	painter.setPen(Qt::black);
			//}

			//// 绘制地址
			//painter.drawText(_vecAddrItem.at(i).rect, Qt::AlignCenter, _vecAddrItem.at(i).text);
		}

		if (_bClicked && addr == _headAddr1) {
			// 绘制点击行的背景色
			QRect rtrt = _vecAddrItem.at(i).rect;
			rtrt.setRect(rtrt.right() + 1, rtrt.y(), 1920, rtrt.height());
			painter.fillRect(rtrt, QColor(245, 245, 245));
		}

		if (addr >= _headAddr1 && addr <= _headAddr2) {
			// 高亮地址
			painter.setPen(QColor("#3399FF"));
		}
		else {
			painter.setPen(Qt::black);
		}

		// 绘制地址
		if (_bDrawRelativeAddress) {
			painter.drawText(_vecAddrItem.at(i).rect, Qt::AlignVCenter | Qt::AlignRight, _vecAddrItem.at(i).text);
		}
		else {
			painter.drawText(_vecAddrItem.at(i).rect, Qt::AlignCenter, _vecAddrItem.at(i).text);
		}
		
	}

	painter.restore();
}

void MemoryView::drawData(QPainter &painter)
{
	painter.save();
	auto hash = _buffer->getHashTable();

	int nSize = _vecDataItem.size();	//数据区域的Item数
	int nByte = 16 / _dataColumnCount;				//一个Item所占用的字节数

	unsigned long long addr = 0;
	int index = 0;
	for (int i = 0; i < nSize; ++i) {
		addr = _firstAddr + i*nByte;	//Item对应的数据地址
		index = i*nByte;				//Item对应的数据下标索引

		// 获取数据
		QString str = getHexData(addr, index);
		_vecDataItem[i].text.swap(str);

		QColor color;
		if (addr >= _dataAddr1 && addr <= _dataAddr2) {
			// 多选
			color.setNamedColor("#FF0000");
			painter.setPen(color);

			// 单选，绘制背景色
			if (_bClicked) {
				int width = _pMetrics->width(_vecDataItem.at(i).text);
				_rtClickedArea.setRect(_vecDataItem.at(i).rect.x(), _vecDataItem.at(i).rect.y(), width, _rowHeight);
				painter.fillRect(_rtClickedArea, QColor(0, 151, 251));		
				color.setNamedColor("#FFFFFF");
				painter.setPen(color);
			}
		}
		else {
			//恢复默认字体和颜色
			painter.setPen(_penColor);
		}
		
		//给变动的数据上背景色色
		//auto it = hash->find(addr);
		//if (it != hash->end()) {
		//	QRect rt(_vecDataItem.at(i).rect);
		//	rt.setRect(rt.x(), rt.y(), _pMetrics->width(_vecDataItem.at(i).text), rt.height());
		//	painter.fillRect(rt, QColor(255, 0, 0, it.value().diaphaneity));

		//	if (color.isValid()) {
		//		color = color.rgb() | QColor(0, it.value().diaphaneity, 0).rgb();
		//	}
		//	else {
		//		color = _penColor;
		//	}
		//	painter.setPen(color);
		//}

		QHash<unsigned long long, RENDER_DATA_INFO>::const_iterator it = hash->begin();
		while (it != hash->end()) {
			if (it.key() >= addr && it.key() < addr + nByte) {
				QRect rt(_vecDataItem.at(i).rect);
				rt.setRect(rt.x(), rt.y(), _pMetrics->width(_vecDataItem.at(i).text), rt.height());
				painter.fillRect(rt, QColor(255, 0, 0, it.value().diaphaneity));

				if (color.isValid()) {
					color = color.rgb() | QColor(0, it.value().diaphaneity, 0).rgb();
				}
				else {
					color = _penColor;
				}
				painter.setPen(color);
				break;
			}
			++it;
		}

		

		// 绘制数据
		painter.drawText(_vecDataItem.at(i).rect, Qt::AlignLeft | Qt::AlignVCenter, _vecDataItem.at(i).text);
	}

	painter.restore();
}

QString MemoryView::getHexData(unsigned long long addr, long long index)
{
	unsigned char* data = _buffer->data();

	bool b = true;	//数据是否可以访问

	if (addr < _mi.endAddr) {
		_penColor = _mi.color;
		b = _mi.state != 0x1000 ? false : true;
	}
	else {
		//大于等于_mi.endAddr说明该地址属于第二块内存
		_penColor = _mi.color2;
		b = _mi.state2 != 0x1000 ? false : true;
	}

	switch (_dataType)
	{
	case e1ByteHex:
		return b ? QString("%1").arg(data[index], 2, 16, QChar('0')).toUpper() : QStringLiteral("??");
	case e2ByteHex:
		return b ? QString("%1").arg(*(unsigned short*)&data[index], 4, 16, QChar('0')).toUpper() : QStringLiteral("????");
	case e4ByteHex:
		return b ? QString("%1").arg(*(unsigned int*)&data[index], 8, 16, QChar('0')).toUpper() : QStringLiteral("????????");
	case e8ByteHex:
		return b ? QString("%1").arg(*(unsigned long long*)&data[index], 16, 16, QChar('0')).toUpper() : QStringLiteral("????????????????");
	case e1ByteDec:
		return b ? _bSymbol ? QString::number(*(char*)&data[index]) : QString::number(data[index]) : QStringLiteral("??");
	case e2ByteDec:
		return b ? _bSymbol ? QString::number(*(short*)&data[index]) : QString::number(*(unsigned short*)&data[index]) : QStringLiteral("????");
	case e4ByteDec:
		return b ? _bSymbol ? QString::number(*(int*)&data[index]) : QString::number(*(unsigned int*)&data[index]) : QStringLiteral("????????");
	case e8ByteDec:
		return b ? _bSymbol ? QString::number(*(long long*)&data[index]) : QString::number(*(unsigned long long*)&data[index]) : QStringLiteral("????????????????");
	case e4ByteFloat: {
		QString str = QString::number(*(float*)&data[index], 'f', 2);
		int width = _pMetrics->width(str);
		int maxWidth = _pMetrics->width(QStringLiteral("2147483648"));
		str = _pMetrics->elidedText(str, Qt::ElideRight, maxWidth);
		return b ? str : QStringLiteral("????????");
	}
	case e8ByteDouble: {
		QString str = QString::number(*(double*)&data[index], 'd', 2);
		int width = _pMetrics->width(str);
		int maxWidth = _pMetrics->width(QStringLiteral("-9223372036854775808"));
		str = _pMetrics->elidedText(str, Qt::ElideRight, maxWidth);
		return b ? str : QStringLiteral("????????????????");
	}
	default:
		break;
	}
	return QString();
}

void MemoryView::drawTextData(QPainter &painter)
{
	painter.save();

	auto hash = _buffer->getHashTable();

	painter.setPen(QColor(0, 128, 54));


	int nSize = _vecTextItem.size();	//文本区域的Item数
	int nByte = 16 / _textColumnCount;	//一个Item所占用的字节数

	unsigned int len;
	unsigned long long addr = 0;
	int index = 0;

	for (int i = 0; i < nSize; ++i) {
		addr = _firstAddr + i*nByte;	//Item对应的数据地址
		index = i*nByte;				//Item对应的数据下标索引

		len = getTextData(addr, index, i, _vecTextItem[i].text);


		if (len > 1) {
			QRect ttt = _vecTextItem.at(i).rect;
			ttt.setRect(ttt.x(), ttt.y(), _textItemWidth * len, _rowHeight);
			_vecTextItem[i].rect = ttt;
		}


		if (addr >= _dataAddr1 && addr <= _dataAddr2) {
			painter.setPen(Qt::red);

			if (_bClicked) {
				painter.fillRect(_vecTextItem.at(i).rect, QColor(0, 151, 251));
				painter.setPen(Qt::white);
			}
		}
		else {
			painter.setPen(_penColor);
		}


		//给变动的数据上背景色色
		auto it = hash->find(addr);
		if (it != hash->end()) {
			QRect rt(_vecTextItem.at(i).rect);
			rt.setRect(rt.x(), rt.y(), _pMetrics->width(_vecTextItem[i].text), rt.height());

			painter.fillRect(rt, QColor(255, 255, 0, it.value().diaphaneity));
			painter.setPen(Qt::red);
		}

		painter.drawText(_vecTextItem.at(i).rect, Qt::AlignLeft | Qt::AlignVCenter, _vecTextItem[i].text);

		i = --i + len;
	}

	painter.restore();
}

unsigned char MemoryView::u8bytes(unsigned char byte)
{
	int bytes = 0;

	if (byte <= 0x7F) { //then ASCII 占用1个字节
		bytes = 1;
	}
	else if (byte >= 0xC0 && byte <= 0xDF) {  // then 首字节   UTF-8 占用2个字节
		bytes = 2;
	}
	else if (byte >= 0xE0 && byte <= 0xEF) {  // then 首字节   UTF-8 占用3个字节
		bytes = 3;
	}
	else if (byte >= 0xF0 && byte <= 0xF7) {  // then 首字节   UTF-8 占用4个字节
		bytes = 4;
	}
	else if (byte >= 0xF8 && byte <= 0xFB) {  // then 首字节   UTF-8 占用5个字节
		bytes = 5;
	}
	else if (byte >= 0xFC && byte <= 0xFD) {  // then 首字节   UTF-8 占用6个字节
		bytes = 6;
	}
	else if (byte > 0x7F && byte < 0xC0) {   // then UTF-8   非首字节
		bytes = 0;
	}

	return bytes;
}
int MemoryView::preNum(unsigned char byte)
{
	unsigned char mask = 0x80;
	int num = 0;
	for (int i = 0; i < 8; i++) {
		if ((byte & mask) == mask) {
			mask = mask >> 1;
			num++;
		}
		else {
			break;
		}
	}
	return num;
}
bool MemoryView::isUtf8Code(const char* data)
{
	int len = strlen(data);
	auto temp = reinterpret_cast <const unsigned char*> (data);
	int num = 0;
	int i = 0;
	while (i < len) {
		if ((temp[i] & 0x80) == 0x00) {
			// 0XXX_XXXX
			i++;
			continue;
		}
		else if ((num = preNum(temp[i])) > 2) {
			// 110X_XXXX 10XX_XXXX
			// 1110_XXXX 10XX_XXXX 10XX_XXXX
			// 1111_0XXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
			// 1111_10XX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
			// 1111_110X 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX 10XX_XXXX
			// preNUm() 返回首个字节8个bits中首�?0bit前面1bit的个数，该数量也是该字符所使用的字节数        
			i++;
			for (int j = 0; j < num - 1; j++) {
				//判断后面num - 1 个字节是不是都是10开
				if ((temp[i] & 0xc0) != 0x80) {
					return false;
				}
				i++;
			}
		}
		else {
			//其他情况说明不是utf-8
			return false;
		}
	}
	return true;
}

unsigned int MemoryView::getTextData(unsigned long long addr, long long index, int &itemIndex, QString &text)
{
	unsigned char* data = _buffer->data();

	if (addr < _mi.endAddr) {
		_penColor = _mi.color;
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
		unsigned char ch = data[index];
		if (iscntrl(ch)) {
			ch = '.';	//是控制符符以.表示
		}
		else if (!isprint(ch)) {
			ch = ' ';	//不可打印的字符以 表示
		}
		text = QChar(ch);
		break;
	}
	case eUTF8: {
		len = u8bytes(data[index]);

		char u8[7]{};
		memcpy(u8, &data[index], len);

		if (len > 1 && isUtf8Code(u8)) {
			text = u8;
		}
		else {
			unsigned char ch = data[index];
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
		char16_t wch = *(char16_t*)&data[index];
		char16_t wch2 = *(char16_t*)&data[index + 2];

		if ((wch >= 0x0000 && wch <= 0xD7FF) || (wch >= 0xE000 && wch <= 0xFFFF)) {
			if (iswcntrl(wch)) {
				wch = u'.';
			}
			text = QString::fromUtf16(&wch, 1); 
			len = 2;
		}
		else if ((wch >= 0xD800 && wch <= 0xDBFF) && (wch2 >= 0xDC00 && wch2 <= 0xDFFF)) {
			wch = (wch & 0x03FF) << 10;
			wch2 = (wch2 & 0x03FF);
			char32_t wch3 = (wch | wch2) + 0x10000;

			//wch = (wch - 0xD800) << 10;
			//wch2 = (wch2 - 0xDC00);
			//char32_t wch3 = (wch + wch2) + 0x10000;


			text = QString::fromUtf16((char16_t*)&wch3, 2);
			len = 4;
		}
		else {
			//wch = L' ';
			text = QString::fromUtf16(&wch, 1);
			len = 2;
		}

		break;
	}
	default:
		break;
	}
	return len;
}

void MemoryView::drawDataHead(QPainter &painter)
{
	painter.save();

	painter.setPen(Qt::black);

	QRect rtTemp;
	QString str;
	int n = 16 / _dataColumnCount;
	for (int i = 0; i < _dataColumnCount; ++i) {
		str = QString("%1").arg((unsigned char)(i*n), 2, 16, QChar('0')).toUpper();

		if (_bClicked && (i*n) == _colIndex) {

			//if (_dataType == e1ByteHex) {
			//	// 画点击的背景
			//	ttt = _vecDataItem.at(i);
			//	ttt.setRect(ttt.x(), _rtOffsetArea.bottom() + 1, _pFontMetrics->width(str), event->rect().height());
			//	painter.fillRect(ttt, QColor(245, 245, 245));
			//}

			painter.setPen(QColor("#3399FF"));
		}
		else {
			painter.setPen(Qt::black);
		}


		rtTemp = _vecDataItem.at(i).rect;
		rtTemp.setRect(rtTemp.x(), _rtOffsetArea.y(), rtTemp.width(), rtTemp.height());
		painter.drawText(rtTemp, Qt::AlignLeft | Qt::AlignVCenter, str);
	}

	painter.restore();
}

void MemoryView::drawTextDataHead(QPainter &painter)
{
	painter.save();

	QRect rtTemp;
	QString str;
	int n = 16 / _textColumnCount;
	for (int i = 0; i < _textColumnCount; ++i) {

		if (_bClicked && (i*n) == _colIndex) {
			painter.setPen(QColor("#3399FF"));
		}
		else {
			painter.setPen(Qt::black);
		}

		str = QString("%1").arg(i*n, 1, 16, QChar('0')).toUpper();
		rtTemp = _vecTextItem.at(i).rect;
		rtTemp.setRect(rtTemp.x(), _rtOffsetArea.y(), rtTemp.width(), rtTemp.height());
		painter.drawText(rtTemp, Qt::AlignLeft | Qt::AlignVCenter, str);
	}

	painter.restore();
}

void MemoryView::drawDivisionLine(QPainter &painter)
{
	painter.save();

	QRect rtTemp;
	int x = _firstAddr % _divisionByte;	//地址对齐
	if (_dataType == e1ByteHex || _dataType == e1ByteDec) {
		for (int i = 0; i < 16; ++i) {
			if (i && (i + x - _divisionByte) % _divisionByte == 0) {
				// 画一条黄色分隔符
				painter.setPen(QColor(230, 230, 0));
				rtTemp = _vecDataItem.at(i).rect;
				painter.drawLine(rtTemp.x() - 4, _strProtectItem.rect.bottom(), rtTemp.x() - 4, 1080);
			}
		}
	}

	painter.restore();
}

void MemoryView::mousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton) {
		//重置标志
		_bPress = true;
		_bClicked = false;
		auto pos = event->pos();
		setBeginPos(pos);
	}
}

void MemoryView::mouseReleaseEvent(QMouseEvent * event)
{
	_bPress = false;

	if (event->button() == Qt::RightButton) {
		// 按下和弹起指向的数据对应的实际地址都一样，则视为点击，否则视为多选
		if (_dataAddr1 == _dataAddr2) {
			auto pos = event->pos();
			setBeginPos(pos);
			_bClicked = true;
		}
		_pMenu->exec(QCursor::pos());
	}
	if (event->button() == Qt::LeftButton) {
		// 按下和弹起指向的数据对应的实际地址都一样，则视为点击，否则视为多选
		if (_dataAddr1 == _dataAddr2) {
			_bClicked = true;
			viewport()->update();
		}
	}
}

void MemoryView::mouseMoveEvent(QMouseEvent * event)
{
	if (_bPress) {
		auto pos = event->pos();
		setEndPos(pos);
	}
}

void MemoryView::mouseDoubleClickEvent(QMouseEvent *event) //鼠标双击事件
{
	if (event->buttons() & Qt::LeftButton) {
		if (_rtDataArea.contains(event->pos())) {
			QVector<HexEditItem>* pVec = &_vecDataItem;
			int n = 16 / _dataColumnCount;
			for (int i = 0; i < pVec->size(); ++i) {
				if (pVec->at(i).rect.contains(event->pos())) {
					_dataIndex1 = _dataIndex2 = i*n;
					_dataAddr1 = _firstAddr + i*n;
					_dataAddr2 = _dataAddr1 + n - 1;

					//判断点击的地址是否可以读取
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

					//读取数据准备修改
					memset(&_dataInfo, 0, sizeof(DataInfo));
					memcpy(_dataInfo.data, &_buffer->data()[_dataIndex1], 128);
					_dataInfo.pid = _pid;
					_dataInfo.beginAddr = _dataAddr1;
					_dataInfo.endAddr = _dataAddr2;
					_dataInfo.bSymbol = _bSymbol;
					_dataInfo.dataType = _dataType;

					_pEditor->show(_dataInfo, eEditorData);

					break;
				}
			}
		}
		else if (_strAllocationBaseAddrItem.rect.contains(event->pos())) {
			_firstAddr = _mi.allocationBase;
			_mi = _buffer->changeAddr(_firstAddr);
			viewport()->update();
		}
		else if (_strBaseAddrItem.rect.contains(event->pos())) {
			_firstAddr = _mi.beginAddr;
			_mi = _buffer->changeAddr(_firstAddr);
			viewport()->update();
		}
		else if (_strAddrSizeItem.rect.contains(event->pos())) {
			_firstAddr = _mi.endAddr;
			_mi = _buffer->changeAddr(_firstAddr);
			viewport()->update();
		}
		else if (_rtAddrArea.contains(event->pos())) {	//显示相对地址
			QVector<HexEditItem>* pVec = &_vecAddrItem;
			for (int i = 0; i < pVec->size(); ++i) {
				if (pVec->at(i).rect.contains(event->pos())) {
					_bDrawRelativeAddress = !_bDrawRelativeAddress;
					_relativeAddress = _firstAddr + i * 0x10;
					break;
				}
			}
			setItem();
			viewport()->update();
		}
	}
}

void MemoryView::setBeginPos(QPoint &pos)
{
	QVector<HexEditItem>* pItems;
	int nCols = 0;
	int nByte = 0;
	if (_rtDataArea.contains(pos)) {
		pItems = &_vecDataItem;
		nCols = _dataColumnCount;
		_releaseArea = 1;
	}
	else if (_rtTextArea.contains(pos)) {
		pItems = &_vecTextItem;
		nCols = _textColumnCount;
		_releaseArea = 2;
	}
	else {
		return;
	}

	int n = 16 / nCols;	//当前数据类型所占的字节数
	for (int i = 0; i < pItems->size(); ++i) {
		if (pItems->at(i).rect.contains(pos)) {

			_dataIndex1_backup = _dataIndex1 = _dataIndex2 = i*n;						//数据缓冲区索引
			_dataAddr1_backup = _dataAddr1 = _dataAddr2 = _firstAddr + _dataIndex1;		//数据的实际地址
			_headAddr1_backup = _headAddr1 = _headAddr2 = _firstAddr + (int)((_dataAddr1 - _firstAddr) / 16) * 16;	//每行的第一个数据对应的实际地址
			_itemIndex1_backup = _itemIndex1 = _itemIndex2 = i;
			_colIndex = i % nCols * n;		//Item所在的列号
			_pLastItem = &pItems->at(i);	//最后一次鼠标指向的Item

			if (_releaseArea == 2) {
				nByte = pItems->at(i).rect.width() / _textItemWidth - 1;
				_dataIndex2 += nByte;
				_dataAddr2 += nByte;
				i = --i + nByte;
			}
			
			viewport()->update();

			//点击读取数据，更新状态栏
			memset(&_dataInfo, 0, sizeof(DataInfo));
			memcpy(_dataInfo.data, &_buffer->data()[_dataIndex1], 128);
			_dataInfo.pid = _pid;
			_dataInfo.beginAddr = _dataAddr1;
			_dataInfo.endAddr = _dataAddr2;
			_dataInfo.byteNum = _dataInfo.endAddr - _dataInfo.beginAddr + 1;
			_dataInfo.dataType = _dataType;
			_dataInfo.bSymbol = _bSymbol;

			emit signal_clickedItem(_dataInfo);

			break;
		}
	}
}
void MemoryView::setEndPos(QPoint &pos)
{
	QVector<HexEditItem>* pItems;
	int nCols = 0;
	int nByte = 0;
	if (_pLastItem && _pLastItem->rect.contains(pos)) {
		return;
	}
	else if (_rtDataArea.contains(pos)) {
		pItems = &_vecDataItem;
		nCols = _dataColumnCount;
		_releaseArea = 1;
	}
	else if (_rtTextArea.contains(pos)) {
		pItems = &_vecTextItem;
		nCols = _textColumnCount;
		_releaseArea = 2;
	}
	else {
		return;
	}

	int n = 16 / nCols;
	int index = 0;
	for (int i = 0; i < pItems->size(); ++i) {
		if (pItems->at(i).rect.contains(pos)) {

			_dataIndex2 = i*n;														//数据缓冲区索引
			_dataAddr2 = _firstAddr + _dataIndex2;									//数据的实际地址
			_headAddr2 = _firstAddr + (int)((_dataAddr2 - _firstAddr) / 16) * 16;	//每行的第一个数据对应的实际地址
			_itemIndex2 = i;
			_pLastItem = &pItems->at(i);		//鼠标指向的Item

			if (_releaseArea == 2) {
				nByte = pItems->at(i).rect.width() / _textItemWidth - 1;
				_dataIndex2 = i*n + nByte;
				_dataAddr2 = _firstAddr + _dataIndex2;
				i = --i + nByte;
			}
			

			break;
		}
	}

	//是否需要交换地址
	_dataAddr1 = _dataAddr1_backup;
	_headAddr1 = _headAddr1_backup;
	_dataIndex1 = _dataIndex1_backup;
	_itemIndex1 = _itemIndex1_backup;
	if (_dataAddr2 < _dataAddr1) {
		_dataAddr2 -= nByte;
		_dataIndex2 -= nByte;
		std::swap(_dataAddr1, _dataAddr2);
		std::swap(_headAddr1, _headAddr2);
		std::swap(_dataIndex1, _dataIndex2);
		std::swap(_itemIndex1, _itemIndex2);
		_dataAddr2 += nByte;
		_dataIndex2 += nByte;
	}



	//点击读取数据，更新状态栏
	memset(&_dataInfo, 0, sizeof(DataInfo));
	memcpy(_dataInfo.data, &_buffer->data()[_dataIndex1], 128);
	_dataInfo.pid = _pid;
	_dataInfo.beginAddr = _dataAddr1;
	_dataInfo.endAddr = _dataAddr2;
	_dataInfo.byteNum = _dataInfo.endAddr - _dataInfo.beginAddr + 1;
	_dataInfo.dataType = _dataType;
	_dataInfo.bSymbol = _bSymbol;

	//更新状态栏
	emit signal_clickedItem(_dataInfo);

	viewport()->update();
}

