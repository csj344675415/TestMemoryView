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
		//_dataAddr1 = 0;		//��갴��ʱ��ָ���Item���ŵ����ݶ�Ӧ�ĵ�ַ
		//_dataAddr2 = 0;		//����ƶ�����ʱ��ָ���Item���ŵ����ݶ�Ӧ�ĵ�ַ
		//_headAddr1 = 0;		//��갴��ʱ��ָ���Item�����е�ͷ��ַ
		//_headAddr2 = 0;		//����ƶ�����ʱ��ָ���Item�����е�ͷ��ַ
		//_dataIndex1 = 0;			//��갴��ʱ��ָ���Item���ŵ�����λ�ڻ������������
		//_dataIndex2 = 0;			//����ƶ�����ʱ��ָ���Item���ŵ�����λ�ڻ������������
		//_itemIndex1 = 0;
		//_itemIndex2 = 0;
		//_colIndex = 0;				//�����ʱ��ָ���Item��Ӧ��������
	}
	
	if (addr == -1) {
		if (_firstAddr == -1) {
			//ģ�������Ϣ
			g_moduleInfo = prc->get��ȡģ����Ϣ(pid);
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
	
	// ��ѯ��ַ�ռ����ڴ��ַ����Ϣ
	//g_hProc = prc->openProsess(pid);
	g_hProc = ::OpenProcess(PROCESS_ALL_ACCESS, false, pid);

	// �ڴ���Ϣ
	MEMORY_BASIC_INFORMATION mbi{};
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	MEMORY_BASIC_INFORMATION mbi2{};
	memset(&mbi2, 0, sizeof(MEMORY_BASIC_INFORMATION));

	::VirtualQueryEx(g_hProc, (LPCVOID)addr, &mbi, sizeof(mbi));


	DWORD_PTR endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;

	// ȡ����ͼ��һ�е�ַ�뵱ǰ�ڴ�������ַ�Ĵ�С
	DWORD dwRemainingSize = endAddr - addr;

	// ��ͼ���Ƿ����2���ڴ�
	bool bMore = false;

	// ���ʣ���С < ����ֽ���
	if (dwRemainingSize < _maxBytes) {
		bMore = true;
		// ��ѯ��һ���ڴ���Ϣ
		::VirtualQueryEx(g_hProc, (LPCVOID)endAddr, &mbi2, sizeof(mbi2));
		_mi.state2 = mbi2.State;
	}

	// 
	_mi.allocationBase = (DWORD_PTR)mbi.AllocationBase;
	_mi.size = mbi.RegionSize;										//��ǰ�ڴ�ҳ�Ĵ�С
	_mi.beginAddr = (DWORD_PTR)mbi.BaseAddress;					//��ǰ�ڴ�ҳ�Ŀ�ʼ��ַ
	_mi.endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;	//��ǰ�ڴ�ҳ�Ľ�����ַ
	_mi.state = mbi.State;
	_mi.type = mbi.Type;
	_mi.protect = mbi.Protect;
	_mi.isBase = false;
	_mi.color = QColor("#000000");
	_mi.color2 = QColor("#000000");

	// ���һ�е�ַ��ƫ��
	_dataOffset = 0;
	// Ҫ��ȡ�����ݴ�С
	_dataSize = _maxBytes;

	// �Ƿ�Ϊ�ѷ���������ڴ棬������Ҫ��ȡ�ĵ�ַƫ�ƺʹ�С
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

	// ���ýṹ���Ա
	// �ж������Ƿ��ڻ�ַ��Χ
	g_vecModuleInfo = prc->enumö��ģ��(pid);
	_mi.moduleName.clear();
	for (auto t : g_vecModuleInfo) {
		if ((DWORD_PTR)mbi.AllocationBase == (DWORD_PTR)t.moduleBaseAddr) {
			_mi.color = QColor("#008036");	//��ɫ
			_mi.isBase = true;
			_mi.moduleName = QStringLiteral("ģ��=") + QString::fromStdWString(t.moduleName);
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
			_mi.strProtect = QStringLiteral("ֻ��");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READ) {
			_mi.strProtect = QStringLiteral("ִ��/��");
		}
		else if (mbi.Protect == PAGE_READWRITE) {
			_mi.strProtect = QStringLiteral("��д");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READWRITE) {
			_mi.strProtect = QStringLiteral("ִ��/��д");
		}
	}
	else {
		_mi.strProtect = QStringLiteral("���ɷ���");
	}

	


	// ��ȡ����
	prc->mem��(addr + _dataOffset, &_pData[_dataOffset], _dataSize);

	_firstAddr = addr;

	updateStart();
}

void HexEdit::changeAddr(unsigned long long addr)
{
	// �ڴ���Ϣ
	MEMORY_BASIC_INFORMATION mbi{};
	memset(&mbi, 0, sizeof(MEMORY_BASIC_INFORMATION));
	MEMORY_BASIC_INFORMATION mbi2{};
	memset(&mbi2, 0, sizeof(MEMORY_BASIC_INFORMATION));

	//g_hProc = prc->openProsess(_pid);
	::VirtualQueryEx(g_hProc, (LPCVOID)addr, &mbi, sizeof(mbi));


	DWORD_PTR endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;

	// ȡ����ͼ��һ�е�ַ�뵱ǰ�ڴ�������ַ�Ĵ�С
	DWORD dwRemainingSize = endAddr - addr;

	// ��ͼ���Ƿ����2���ڴ�
	bool bMore = false;

	// ���ʣ���С < ����ֽ���
	if (dwRemainingSize < _maxBytes) {
		bMore = true;
		// ��ѯ��һ���ڴ���Ϣ
		::VirtualQueryEx(g_hProc, (LPCVOID)endAddr, &mbi2, sizeof(mbi2));
		_mi.state2 = mbi2.State;
	}

	// 
	_mi.allocationBase = (DWORD_PTR)mbi.AllocationBase;
	_mi.size = mbi.RegionSize;										//��ǰ�ڴ�ҳ�Ĵ�С
	_mi.beginAddr = (DWORD_PTR)mbi.BaseAddress;					//��ǰ�ڴ�ҳ�Ŀ�ʼ��ַ
	_mi.endAddr = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;	//��ǰ�ڴ�ҳ�Ľ�����ַ
	_mi.state = mbi.State;
	_mi.type = mbi.Type;
	_mi.protect = mbi.Protect;
	_mi.isBase = false;
	_mi.color = QColor("#000000");
	_mi.color2 = QColor("#000000");

	// ���һ�е�ַ��ƫ��
	_dataOffset = 0;
	// Ҫ��ȡ�����ݴ�С
	_dataSize = _maxBytes;

	// �Ƿ�Ϊ�ѷ���������ڴ棬������Ҫ��ȡ�ĵ�ַƫ�ƺʹ�С
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

	// ���ýṹ���Ա
	// �ж������Ƿ��ڻ�ַ��Χ
	_mi.moduleName.clear();
	for (auto t : g_vecModuleInfo) {
		if ((DWORD_PTR)mbi.AllocationBase == (DWORD_PTR)t.moduleBaseAddr) {
			_mi.color = QColor("#008036");	//��ɫ
			_mi.isBase = true;
			_mi.moduleName = QStringLiteral("ģ��=") + QString::fromStdWString(t.moduleName);
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
			_mi.strProtect = QStringLiteral("ֻ��");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READ) {
			_mi.strProtect = QStringLiteral("ִ��/��");
		}
		else if (mbi.Protect == PAGE_READWRITE) {
			_mi.strProtect = QStringLiteral("��д");
		}
		else if (mbi.Protect == PAGE_EXECUTE_READWRITE) {
			_mi.strProtect = QStringLiteral("ִ��/��д");
		}
	}
	else {
		_mi.strProtect = QStringLiteral("���ɷ���");
	}

	// ��ȡ����
	prc->mem��(addr + _dataOffset, &_pData[_dataOffset], _dataSize);

	_firstAddr = addr;
}

void HexEdit::updateStop()
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

void HexEdit::updateStart()
{
	if (_nTimerID2 == -1) {
		_nTimerID2 = this->startTimer(_interval);
	}

	//�����̣߳���������	
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

		// �ڴ���Ϣ
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
		//prc->mem��(_firstAddr + _dataOffset, &tmpData[_dataOffset], _dataSize);
		ReadProcessMemory(g_hProc, (LPVOID)(_firstAddr + _dataOffset), &tmpData[_dataOffset], _dataSize, NULL);

		for (int i = _dataOffset; i < _dataSize; ++i) {

			if (tmpData[i] != _pData[i]) {
				unsigned long long index = _firstAddr + i;

				RENDER_DATA_INFO rdi{};
				rdi.ms = _ms;
				rdi.diaphaneity = 255;

				_hashAddrTable.insert(index, rdi);	//���hash�����Ѵ��ڸ�������index�������Զ����¶�Ӧ��ֵΪ��_ms��

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

void HexEdit::timerEvent(QTimerEvent *event) //�����¼�
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
		//QMutexLocker locker(&_mutex); //����˳������ 

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