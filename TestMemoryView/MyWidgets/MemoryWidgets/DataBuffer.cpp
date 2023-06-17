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

	HANDLE hModuleSnapshot = INVALID_HANDLE_VALUE;                      //ģ����վ��
	hModuleSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pid);	//��ȡָ�����̵�ģ����գ�����2.����ID
	if (hModuleSnapshot == INVALID_HANDLE_VALUE) return;

	//���ýṹ��
	MODULEENTRY32W me32;                      //ģ��ṹ��
	memset(&me32, 0, sizeof(MODULEENTRY32W)); //��ʼ���ṹ���ÿ�
	me32.dwSize = sizeof(MODULEENTRY32W);     //���ýṹ���С

											  //��ʼ����  
	if (FALSE == Module32First(hModuleSnapshot, &me32)) {//��ȡ��һ��ģ����Ϣ
		CloseHandle(hModuleSnapshot); //�رտ��վ��
		return;
	}

	_vecModule.clear();
	do {
		QMODULE_INFO_T mi;
		mi.moduleName = QString::fromWCharArray(me32.szModule);		//ģ����
		mi.modulePath = QString::fromWCharArray(me32.szExePath);	//ģ��·��
		mi.moduleBaseAddr = (unsigned long long)me32.hModule;		//���ػ�ַ
		mi.moduleSize = (unsigned long long)me32.modBaseSize;		//��С	
		mi.gcount = me32.GlblcntUsage;								//ȫ�����ü���
		mi.count = me32.ProccntUsage;								//�������ü���
		_vecModule.append(mi);
	} while (Module32Next(hModuleSnapshot, &me32)); //��ȡ��һ��ģ����Ϣ

													//��β����
	CloseHandle(hModuleSnapshot); //�رտ��վ��
}

MEM_INFO_T DataBuffer::setData(unsigned int pid, unsigned long long addr)
{
	_pid = pid;
	getModuleInfo(pid);
	
	auto ret = changeAddr(addr);

	updateStart();	//��ʼ����ˢ��

	return ret;
}

MEM_INFO_T DataBuffer::changeAddr(unsigned long long addr)
{
	if (addr == -1) {
		if (_firstAddr == -1) {
			if (!_vecModule.isEmpty()) {
				addr = _vecModule.at(0).moduleBaseAddr;	//0 = exeģ�飬����0Ϊdllģ��
			}
			else {
				//��ȡ����ģ����Ϣ
				return MEM_INFO_T();
			}
		}
		else {
			addr = _firstAddr;
		}
	}

	// �ڴ���Ϣ
	MEMORY_BASIC_INFORMATION mbi{};
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	MEMORY_BASIC_INFORMATION mbi2{};
	memset(&mbi2, 0, sizeof(MEMORY_BASIC_INFORMATION));

	if (::VirtualQueryEx(g_hProcess, (LPCVOID)addr, &mbi, sizeof(mbi)) != sizeof(mbi)) {
		//��ȡ�ڴ���Ϣʧ��
		return MEM_INFO_T();
	}

	// �ڴ��Ľ�����ַ
	DWORD_PTR endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;

	// ȡ����ͼ��һ�е�ַ���������ַ�Ĵ�С
	DWORD dwRemainingSize = endAddr - addr;

	// ��ͼ���Ƿ����2���ڴ�
	bool b = false;

	// ���ʣ���С < ����ֽ���
	if (dwRemainingSize < _maxBytes) {
		b = true;
		// ��ѯ��һ���ڴ���Ϣ
		::VirtualQueryEx(g_hProcess, (LPCVOID)endAddr, &mbi2, sizeof(mbi2));
		_strMI.state2 = mbi2.State;
	}

	// 
	_strMI.firstAddr = addr;
	_strMI.allocationBase = (DWORD_PTR)mbi.AllocationBase;
	_strMI.size = mbi.RegionSize;										//��ǰ�ڴ�ҳ�Ĵ�С
	_strMI.beginAddr = (DWORD_PTR)mbi.BaseAddress;					//��ǰ�ڴ�ҳ�Ŀ�ʼ��ַ
	_strMI.endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;	//��ǰ�ڴ�ҳ�Ľ�����ַ
	_strMI.state = mbi.State;
	_strMI.type = mbi.Type;
	_strMI.protect = mbi.Protect;
	_strMI.color = QColor("#000000");
	_strMI.color2 = QColor("#000000");

	// ���һ�е�ַ��ƫ��
	_dataOffset = 0;
	// Ҫ��ȡ�����ݴ�С
	_dataSize = _maxBytes;

	// �Ƿ�Ϊ�ѷ���������ڴ棬������Ҫ��ȡ�ĵ�ַƫ�ƺʹ�С
	if (mbi.State == MEM_COMMIT) {
		if (b) {
			if (mbi2.State == MEM_COMMIT) {
				// 2���ڴ涼�ѷ������ȡ��ͼ����ʾ������ֽ���
				_dataSize = _maxBytes;
			}
			else {
				// ֻ��ȡ��һ���ڴ��ʣ������
				_dataSize = dwRemainingSize;
			}
		}
	}
	else if (mbi.State != MEM_COMMIT) {
		if (b) {
			if (mbi2.State == MEM_COMMIT) {
				// ƫ��ָ���2���ڴ棬ֻ��ȡ�ѷ���ĵ�2���ڴ�����
				_dataOffset = dwRemainingSize;
				_dataSize = _maxBytes - dwRemainingSize;
			}
			else {
				// ����2���ڴ��Ҷ�δ���������ڴ�ʱ������ȡ�κ�����
				_dataOffset = 0;
				_dataSize = 0;
			}
		}
	}

	// ���ýṹ���Ա
	// �ж������Ƿ��ڻ�ַ��Χ
	_strMI.moduleName.clear();
	
	for (auto t : _vecModule) {
		if ((DWORD_PTR)mbi.AllocationBase == (DWORD_PTR)t.moduleBaseAddr) {
			_strMI.color = QColor("#008000");
			_strMI.moduleName = QStringLiteral("ģ��=") + t.moduleName;
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
			_strMI.strProtect = QStringLiteral("ֻ��");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READ) {
			_strMI.strProtect = QStringLiteral("ִ��/��");
		}
		else if (mbi.Protect == PAGE_READWRITE) {
			_strMI.strProtect = QStringLiteral("��/д");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READWRITE) {
			_strMI.strProtect = QStringLiteral("ִ��/��д");
		}
	}
	else {
		_strMI.strProtect = QStringLiteral("���ɷ���");
	}


	// ��ȡ����
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

		// �ڴ���Ϣ
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

				_hashAddrTable.insert(index, rdi);	//���hash�����Ѵ��ڸ�������index�������Զ����¶�Ӧ��ֵΪ��_ms��

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
	killTimer(_nTimerID2);
	_nTimerID2 = -1;
	_hashAddrTable.clear();	//��չ�ϣ��
}

void DataBuffer::updateStart()
{
	if (_nTimerID2 == -1) {
		_nTimerID2 = this->startTimer(_interval);
	}

	//�����̣߳���������	
	if (!_t) {
		_bStrat_t = true;
		_t = new std::thread(&DataBuffer::_tUpdateData, this);
	}
}

void DataBuffer::timerEvent(QTimerEvent *event) //�����¼�
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