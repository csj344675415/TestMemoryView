#include "DataBuffer.h"
#include <QTimerEvent>

#include <windows.h>
#include <shellapi.h>
#include <tlhelp32.h>
#include <Psapi.h>
#pragma comment (lib,"Psapi.lib")

HANDLE g_hProcess = 0;

DataBuffer::DataBuffer(QObject *parent)
	: QObject(parent)
{
}

DataBuffer::~DataBuffer()
{
	updateStop();
	if (g_hProcess != 0) {
		CloseHandle(g_hProcess);
		g_hProcess = 0;
	}
}

void DataBuffer::getModuleInfo(unsigned int pid)
{
	if (pid == 0) return;

	if (g_hProcess != 0) {
		CloseHandle(g_hProcess);
		g_hProcess = 0;
	}
	g_hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, false, _pid);

	HANDLE hModuleSnapshot = INVALID_HANDLE_VALUE;                      //模块快照句柄
	hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);	//获取指定进程的模块快照，参数2.进程ID
	if (hModuleSnapshot == INVALID_HANDLE_VALUE) return;

	//设置结构体
	MODULEENTRY32W me32;                      //模块结构体
	memset(&me32, 0, sizeof(MODULEENTRY32W)); //初始化结构体置空
	me32.dwSize = sizeof(MODULEENTRY32W);     //设置结构体大小

											  //开始遍历  
	if (FALSE == Module32First(hModuleSnapshot, &me32)) {//获取第一个模块信息
		CloseHandle(hModuleSnapshot); //关闭快照句柄
		return;
	}

	_vecModule.clear();
	do {
		QMODULE_INFO_T mi;
		mi.moduleName = QString::fromWCharArray(me32.szModule);		//模块名
		mi.modulePath = QString::fromWCharArray(me32.szExePath);	//模块路径
		mi.moduleBaseAddr = (unsigned long long)me32.hModule;		//加载基址
		mi.moduleSize = (unsigned long long)me32.modBaseSize;		//大小	
		mi.gcount = me32.GlblcntUsage;								//全局引用计数
		mi.count = me32.ProccntUsage;								//进程引用计数
		_vecModule.append(mi);
	} while (Module32Next(hModuleSnapshot, &me32)); //获取下一个模块信息

													//收尾工作
	CloseHandle(hModuleSnapshot); //关闭快照句柄
}

MEM_INFO_T DataBuffer::setData(unsigned int pid, unsigned long long addr)
{
	_pid = pid;
	getModuleInfo(pid);
	
	auto ret = changeAddr(addr);

	updateStart();	//开始数据刷新

	return ret;
}

MEM_INFO_T DataBuffer::changeAddr(unsigned long long addr)
{
	if (addr == -1) {
		if (_firstAddr == -1) {
			if (!_vecModule.isEmpty()) {
				addr = _vecModule.at(0).moduleBaseAddr;	//0 = exe模块，大于0为dll模块
			}
			else {
				//获取不到模块信息
				return MEM_INFO_T();
			}
		}
		else {
			addr = _firstAddr;
		}
	}

	// 内存信息
	MEMORY_BASIC_INFORMATION mbi{};
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	MEMORY_BASIC_INFORMATION mbi2{};
	memset(&mbi2, 0, sizeof(MEMORY_BASIC_INFORMATION));

	if (::VirtualQueryEx(g_hProcess, (LPCVOID)addr, &mbi, sizeof(mbi)) != sizeof(mbi)) {
		//获取内存信息失败
		return MEM_INFO_T();
	}

	// 内存块的结束地址
	DWORD_PTR endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;

	// 取得视图第一行地址距离结束地址的大小
	DWORD dwRemainingSize = endAddr - addr;

	// 视图中是否存在2块内存
	bool b = false;

	// 如果剩余大小 < 最大字节数
	if (dwRemainingSize < _maxBytes) {
		b = true;
		// 查询下一块内存信息
		::VirtualQueryEx(g_hProcess, (LPCVOID)endAddr, &mbi2, sizeof(mbi2));
		_strMI.state2 = mbi2.State;
	}

	// 
	_strMI.firstAddr = addr;
	_strMI.allocationBase = (DWORD_PTR)mbi.AllocationBase;
	_strMI.size = mbi.RegionSize;										//当前内存页的大小
	_strMI.beginAddr = (DWORD_PTR)mbi.BaseAddress;					//当前内存页的开始地址
	_strMI.endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;	//当前内存页的结束地址
	_strMI.state = mbi.State;
	_strMI.type = mbi.Type;
	_strMI.protect = mbi.Protect;
	_strMI.color = QColor("#000000");
	_strMI.color2 = QColor("#000000");

	// 与第一行地址的偏移
	_dataOffset = 0;
	// 要读取的数据大小
	_dataSize = _maxBytes;

	// 是否为已分配的物理内存，并计算要读取的地址偏移和大小
	if (mbi.State == MEM_COMMIT) {
		if (b) {
			if (mbi2.State == MEM_COMMIT) {
				// 2块内存都已分配则读取视图可显示的最大字节数
				_dataSize = _maxBytes;
			}
			else {
				// 只读取第一块内存的剩余数据
				_dataSize = dwRemainingSize;
			}
		}
	}
	else if (mbi.State != MEM_COMMIT) {
		if (b) {
			if (mbi2.State == MEM_COMMIT) {
				// 偏移指向第2块内存，只读取已分配的第2块内存数据
				_dataOffset = dwRemainingSize;
				_dataSize = _maxBytes - dwRemainingSize;
			}
			else {
				// 存在2块内存且都未分配物理内存时，不读取任何数据
				_dataOffset = 0;
				_dataSize = 0;
			}
		}
	}

	// 设置结构体成员
	// 判断数据是否在基址范围
	_strMI.moduleName.clear();
	
	for (auto t : _vecModule) {
		if ((DWORD_PTR)mbi.AllocationBase == (DWORD_PTR)t.moduleBaseAddr) {
			_strMI.color = QColor("#008000");
			_strMI.moduleName = QStringLiteral("模块=") + t.moduleName;
			if (!b) break;
		}

		if (b && (DWORD_PTR)mbi2.AllocationBase == (DWORD_PTR)t.moduleBaseAddr) {
			_strMI.color2 = QColor("#008000");
			break;
		}
	}

	if (mbi.State == MEM_COMMIT) {

		QString strProtect;
		if (mbi.Protect == PAGE_READONLY) {
			_strMI.strProtect = QStringLiteral("只读");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READ) {
			_strMI.strProtect = QStringLiteral("执行/读");
		}
		else if (mbi.Protect == PAGE_READWRITE) {
			_strMI.strProtect = QStringLiteral("读/写");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READWRITE) {
			_strMI.strProtect = QStringLiteral("执行/读写");
		}
	}
	else {
		_strMI.strProtect = QStringLiteral("不可访问");
	}


	// 读取数据
	b = ReadProcessMemory(g_hProcess, (LPVOID)(addr + _dataOffset), &_buffer[_dataOffset], _dataSize, NULL);

	_firstAddr = addr;

	return _strMI;
}

void DataBuffer::_tUpdateData()
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
		::VirtualQueryEx(g_hProcess, (LPCVOID)(_firstAddr + _dataOffset), &mbi, sizeof(mbi));
		if (mbi.State != MEM_COMMIT) {
			changeAddr(_firstAddr + _dataOffset);
			emit signal_dataChange(); 
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			continue;
		}

		bUpdate = false;
		ReadProcessMemory(g_hProcess, (LPVOID)(_firstAddr + _dataOffset), &tmpData[_dataOffset], _dataSize, NULL);

		for (int i = _dataOffset; i < _dataSize; ++i) {

			if (tmpData[i] != _buffer[i]) {
				unsigned long long index = _firstAddr + i;

				RENDER_DATA_INFO rdi{};
				rdi.ms = _ms;
				rdi.diaphaneity = 255;

				_hashAddrTable.insert(index, rdi);	//如果hash表里已存在该索引（index），则自动更新对应的值为（_ms）

				std::swap(tmpData[i], _buffer[i]);

				bUpdate = true;
			}
		}
		if (bUpdate) emit signal_dataChange();

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	if (tmpData) {
		delete[] tmpData;
		tmpData = nullptr;
	}
	if (g_hProcess != 0) {
		CloseHandle(g_hProcess);
		g_hProcess = 0;
	}
	
}

void DataBuffer::updateStop()
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

void DataBuffer::updateStart()
{
	if (_nTimerID2 == -1) {
		_nTimerID2 = this->startTimer(_interval);
	}

	//开启线程，更新数据	
	if (!_t) {
		_bStrat_t = true;
		_t = new std::thread(&DataBuffer::_tUpdateData, this);
	}
}

void DataBuffer::timerEvent(QTimerEvent *event) //定义事件
{
	if (event->timerId() == _nTimerID2) {
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
				++it;
			}
		}

		if (bUpdate) emit signal_update();
	}

}

void DataBuffer::changeTime(int ms)
{
	_ms = ms;

	QHash<unsigned long long, RENDER_DATA_INFO>::iterator it = _hashAddrTable.begin();
	while (it != _hashAddrTable.end()) {
		it.value().ms += ms - it.value().ms;
		++it;
	}
}