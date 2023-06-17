#include "HexEdit.h"
#include <QScrollBar>

#include "ReuseCode.hpp"
extern ReuseCode *prc;

HANDLE g_hProc = 0;
MODULE_INFO g_moduleInfo{};
std::vector<MODULE_INFO> g_vecModuleInfo;

void HexEdit::setData(unsigned int pid, unsigned long long addr)
{
	if (pid != _pid) {
		_pid = pid;
		if (g_hProc != 0) {
			CloseHandle(g_hProc);
			g_hProc = 0;
		}
		//_firstAddr = addr = -1;
		//_bClicked = false;
		//_dataAddr1 = 0;		//鼠标按下时，指向的Item里存放的数据对应的地址
		//_dataAddr2 = 0;		//鼠标移动或弹起时，指向的Item里存放的数据对应的地址
		//_headAddr1 = 0;		//鼠标按下时，指向的Item所在行的头地址
		//_headAddr2 = 0;		//鼠标移动或弹起时，指向的Item所在行的头地址
		//_dataIndex1 = 0;			//鼠标按下时，指向的Item里存放的数据位于缓冲区里的索引
		//_dataIndex2 = 0;			//鼠标移动或弹起时，指向的Item里存放的数据位于缓冲区里的索引
		//_itemIndex1 = 0;
		//_itemIndex2 = 0;
		//_colIndex = 0;				//鼠标点击时，指向的Item对应的列索引
	}
	
	if (addr == -1) {
		if (_firstAddr == -1) {
			//模块基本信息
			g_moduleInfo = prc->get获取模块信息(pid);
			addr = (unsigned long long)g_moduleInfo.moduleBaseAddr;
		}
		else {
			addr = _firstAddr;
		}	
	}

	//---
	//addr = 0x00FACF20;
	//_headAddr1 = addr;
	//---
	
	// 查询地址空间中内存地址的信息
	//g_hProc = prc->openProsess(pid);
	g_hProc = ::OpenProcess(PROCESS_ALL_ACCESS, false, pid);

	// 内存信息
	MEMORY_BASIC_INFORMATION mbi{};
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	MEMORY_BASIC_INFORMATION mbi2{};
	memset(&mbi2, 0, sizeof(MEMORY_BASIC_INFORMATION));

	::VirtualQueryEx(g_hProc, (LPCVOID)addr, &mbi, sizeof(mbi));


	DWORD_PTR endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;

	// 取得视图第一行地址与当前内存块结束地址的大小
	DWORD dwRemainingSize = endAddr - addr;

	// 视图中是否存在2块内存
	bool bMore = false;

	// 如果剩余大小 < 最大字节数
	if (dwRemainingSize < _maxBytes) {
		bMore = true;
		// 查询下一块内存信息
		::VirtualQueryEx(g_hProc, (LPCVOID)endAddr, &mbi2, sizeof(mbi2));
		_mi.state2 = mbi2.State;
	}

	// 
	_mi.allocationBase = (DWORD_PTR)mbi.AllocationBase;
	_mi.size = mbi.RegionSize;										//当前内存页的大小
	_mi.beginAddr = (DWORD_PTR)mbi.BaseAddress;					//当前内存页的开始地址
	_mi.endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;	//当前内存页的结束地址
	_mi.state = mbi.State;
	_mi.type = mbi.Type;
	_mi.protect = mbi.Protect;
	_mi.isBase = false;
	_mi.color = QColor("#000000");
	_mi.color2 = QColor("#000000");

	// 与第一行地址的偏移
	_dataOffset = 0;
	// 要读取的数据大小
	_dataSize = _maxBytes;

	// 是否为已分配的物理内存，并计算要读取的地址偏移和大小
	if (mbi.State == MEM_COMMIT) {
		if (bMore) {
			if (mbi2.State == MEM_COMMIT) {
				_dataSize = _maxBytes;
			}
			else {
				_dataSize = dwRemainingSize;
			}
		}
	}
	else {
		_dataSize = 0;
		if (bMore) {
			if (mbi2.State == MEM_COMMIT) {
				_dataOffset = dwRemainingSize;
				_dataSize = _maxBytes - dwRemainingSize;
			}
			else {
				_dataOffset = 0;
				_dataSize = 0;
			}
		}
	}

	// 设置结构体成员
	// 判断数据是否在基址范围
	g_vecModuleInfo = prc->enum枚举模块(pid);
	_mi.moduleName.clear();
	for (auto t : g_vecModuleInfo) {
		if ((DWORD_PTR)mbi.AllocationBase == (DWORD_PTR)t.moduleBaseAddr) {
			_mi.color = QColor("#008036");	//绿色
			_mi.isBase = true;
			_mi.moduleName = QStringLiteral("模块=") + QString::fromStdWString(t.moduleName);
			if(!bMore) break;
		}

		if (bMore && (DWORD_PTR)mbi2.AllocationBase == (DWORD_PTR)t.moduleBaseAddr) {
			_mi.color2 = QColor("#008036");
			break;
		}
	}

	if (MEM_COMMIT == mbi.State) {	

		QString strProtect;
		if (mbi.Protect == PAGE_READONLY) {
			_mi.strProtect = QStringLiteral("只读");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READ) {
			_mi.strProtect = QStringLiteral("执行/读");
		}
		else if (mbi.Protect == PAGE_READWRITE) {
			_mi.strProtect = QStringLiteral("读写");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READWRITE) {
			_mi.strProtect = QStringLiteral("执行/读写");
		}
	}
	else {
		_mi.strProtect = QStringLiteral("不可访问");
	}

	


	// 读取数据
	prc->mem读(addr + _dataOffset, &_pData[_dataOffset], _dataSize);

	_firstAddr = addr;

	updateStart();
}

void HexEdit::changeAddr(unsigned long long addr)
{
	// 内存信息
	MEMORY_BASIC_INFORMATION mbi{};
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	MEMORY_BASIC_INFORMATION mbi2{};
	memset(&mbi2, 0, sizeof(MEMORY_BASIC_INFORMATION));

	//g_hProc = prc->openProsess(_pid);
	::VirtualQueryEx(g_hProc, (LPCVOID)addr, &mbi, sizeof(mbi));


	DWORD_PTR endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;

	// 取得视图第一行地址与当前内存块结束地址的大小
	DWORD dwRemainingSize = endAddr - addr;

	// 视图中是否存在2块内存
	bool bMore = false;

	// 如果剩余大小 < 最大字节数
	if (dwRemainingSize < _maxBytes) {
		bMore = true;
		// 查询下一块内存信息
		::VirtualQueryEx(g_hProc, (LPCVOID)endAddr, &mbi2, sizeof(mbi2));
		_mi.state2 = mbi2.State;
	}

	// 
	_mi.allocationBase = (DWORD_PTR)mbi.AllocationBase;
	_mi.size = mbi.RegionSize;										//当前内存页的大小
	_mi.beginAddr = (DWORD_PTR)mbi.BaseAddress;					//当前内存页的开始地址
	_mi.endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;	//当前内存页的结束地址
	_mi.state = mbi.State;
	_mi.type = mbi.Type;
	_mi.protect = mbi.Protect;
	_mi.isBase = false;
	_mi.color = QColor("#000000");
	_mi.color2 = QColor("#000000");

	// 与第一行地址的偏移
	_dataOffset = 0;
	// 要读取的数据大小
	_dataSize = _maxBytes;

	// 是否为已分配的物理内存，并计算要读取的地址偏移和大小
	if (mbi.State == MEM_COMMIT) {
		if (bMore) {
			if (mbi2.State == MEM_COMMIT) {
				_dataSize = _maxBytes;
			}
			else {
				_dataSize = dwRemainingSize;
			}
		}
	}
	else {
		_dataSize = 0;
		if (bMore) {
			if (mbi2.State == MEM_COMMIT) {
				_dataOffset = dwRemainingSize;
				_dataSize = _maxBytes - dwRemainingSize;
			}
			else {
				_dataOffset = 0;
				_dataSize = 0;
			}
		}
	}

	// 设置结构体成员
	// 判断数据是否在基址范围
	_mi.moduleName.clear();
	for (auto t : g_vecModuleInfo) {
		if ((DWORD_PTR)mbi.AllocationBase == (DWORD_PTR)t.moduleBaseAddr) {
			_mi.color = QColor("#008036");	//绿色
			_mi.isBase = true;
			_mi.moduleName = QStringLiteral("模块=") + QString::fromStdWString(t.moduleName);
			if (!bMore) break;
		}

		if (bMore && (DWORD_PTR)mbi2.AllocationBase == (DWORD_PTR)t.moduleBaseAddr) {
			_mi.color2 = QColor("#008036");
			break;
		}
	}

	if (MEM_COMMIT == mbi.State) {

		QString strProtect;
		if (mbi.Protect == PAGE_READONLY) {
			_mi.strProtect = QStringLiteral("只读");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READ) {
			_mi.strProtect = QStringLiteral("执行/读");
		}
		else if (mbi.Protect == PAGE_READWRITE) {
			_mi.strProtect = QStringLiteral("读写");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READWRITE) {
			_mi.strProtect = QStringLiteral("执行/读写");
		}
	}
	else {
		_mi.strProtect = QStringLiteral("不可访问");
	}

	// 读取数据
	prc->mem读(addr + _dataOffset, &_pData[_dataOffset], _dataSize);

	_firstAddr = addr;
}

void HexEdit::updateStop()
{
	// 退出线程
	if (_t) {
		if (_t->joinable()) {
			_bStrat_t = false;
			_t->join();
			delete _t;
			_t = nullptr;
		}
	}

	// 关闭定时器
	killTimer(_nTimerID2);
	_nTimerID2 = -1;
	_hashAddrTable.clear();	//清空哈希表
}

void HexEdit::updateStart()
{
	if (_nTimerID2 == -1) {
		_nTimerID2 = this->startTimer(_interval);
	}

	//开启线程，更新数据	
	if (!_t) {
		_bStrat_t = true;
		_t = new std::thread(&HexEdit::_tUpdateData, this);
	}
}

HANDLE hProc = 0;
void HexEdit::_tUpdateData()
{
	int nDataSize = _maxBytes;
	unsigned char *tmpData = nullptr;
	tmpData = new unsigned char[nDataSize];
	bool bUpdate = false;
	while (_bStrat_t) {
		if (nDataSize < _maxBytes) {
			delete[] tmpData;
			nDataSize = _maxBytes;
			tmpData = new unsigned char[nDataSize];
		}

		// 内存信息
		MEMORY_BASIC_INFORMATION mbi{};
		memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
		::VirtualQueryEx(g_hProc, (LPCVOID)(_firstAddr + _dataOffset), &mbi, sizeof(mbi));
		if (mbi.State != MEM_COMMIT) {
			changeAddr(_firstAddr + _dataOffset);
			//emit signal_updateHexEditView();
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			continue;
		}

		bUpdate = false;
		//prc->mem读(_firstAddr + _dataOffset, &tmpData[_dataOffset], _dataSize);
		ReadProcessMemory(g_hProc, (LPVOID)(_firstAddr + _dataOffset), &tmpData[_dataOffset], _dataSize, NULL);

		for (int i = _dataOffset; i < _dataSize; ++i) {

			if (tmpData[i] != _pData[i]) {
				unsigned long long index = _firstAddr + i;

				RENDER_DATA_INFO rdi{};
				rdi.ms = _ms;
				rdi.diaphaneity = 255;

				_hashAddrTable.insert(index, rdi);	//如果hash表里已存在该索引（index），则自动更新对应的值为（_ms）

				std::swap(tmpData[i], _pData[i]);

				bUpdate = true;
			}
		}
		if (bUpdate) emit signal_updateHexEditView();

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	if (tmpData) {
		delete[] tmpData;
		tmpData = nullptr;
	}
	if (g_hProc != 0) {
		CloseHandle(g_hProc);
		g_hProc = 0;
	}
}

void HexEdit::timerEvent(QTimerEvent *event) //定义事件
{
	if (event->timerId() == _nTimerID) {
		int value = verticalScrollBar()->value();
		if (value > _middleValue) {
			_firstAddr += 0x50;
		}
		else if (value < _middleValue) {
			_firstAddr -= 0x50;
		}
		changeAddr(_firstAddr);
		viewport()->update();
	}
	else if (event->timerId() == _nTimerID2) {
		//QMutexLocker locker(&_mutex); //定义顺便上锁 

		bool bUpdate = false;

		QHash<unsigned long long, RENDER_DATA_INFO>::iterator it = _hashAddrTable.begin();
		while (it != _hashAddrTable.end()) {
			bUpdate = true;
			it.value().ms -= _interval;
			if (it.value().ms <= 0) {
				it = _hashAddrTable.erase(it);
			}
			else {
				float f = (float)it.value().ms / _ms;
				it.value().diaphaneity = (unsigned char)(255 * f);
				//PLOGD << "it.value().diaphaneity = " << it.value().diaphaneity;
				++it;
			}
		}

		if (bUpdate) viewport()->update();
	}
	 
}