#include "MemoryViewCommand.h"

#include "windows.h"

MemoryViewCommand::MemoryViewCommand(COMMADN_PARAMETER cp, QUndoCommand *parent)
	: QUndoCommand(parent)
{
	__pid = cp.pid;
	__addr = cp.addr;
	__size = cp.size;
	memset(__originalBuff, 0, 256);
	memcpy(__originalBuff, cp.originalBuff, cp.size);
	memset(__newBuff, 0, 256);
	memcpy(__newBuff, cp.newBuff, cp.size);

	//__originalBuff = new unsigned char[size]();
	//__newBuff = new unsigned char[size]();
	//memcpy(__originalBuff, buf1, size);
	//memcpy(__newBuff, buf2, size);
}

MemoryViewCommand::~MemoryViewCommand()
{
	//delete[] __originalBuff;
	//delete[] __newBuff;
}

//³·Ïú
void MemoryViewCommand::undo()
{
	HANDLE hProc = ::OpenProcess(PROCESS_ALL_ACCESS, false, __pid);
	bool b = WriteProcessMemory(hProc, (LPVOID)__addr, __originalBuff, __size, NULL);
	CloseHandle(hProc);
}

//ÖØ×ö
void MemoryViewCommand::redo()
{
	HANDLE hProc = ::OpenProcess(PROCESS_ALL_ACCESS, false, __pid);
	bool b = WriteProcessMemory(hProc, (LPVOID)__addr, __newBuff, __size, NULL);
	CloseHandle(hProc);
}






