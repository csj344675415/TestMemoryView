#include "WriteCommand.h"

#include "windows.h"

WriteCommand::WriteCommand(unsigned int pid, unsigned long long addr, unsigned char* buf1, unsigned char* buf2, unsigned int size, QUndoCommand *parent)
	: QUndoCommand(parent)
{
	__pid = pid;
	__addr = addr;
	__size = size;
	memset(__originalBuff, 0, 256);
	memcpy(__originalBuff, buf1, size);
	memset(__newBuff, 0, 256);
	memcpy(__newBuff, buf2, size);

	//__originalBuff = new unsigned char[size]();
	//__newBuff = new unsigned char[size]();
	//memcpy(__originalBuff, buf1, size);
	//memcpy(__newBuff, buf2, size);
}

WriteCommand::~WriteCommand()
{
	//delete[] __originalBuff;
	//delete[] __newBuff;
}

//³·Ïú
void WriteCommand::undo()
{
	HANDLE hProc = ::OpenProcess(PROCESS_ALL_ACCESS, false, __pid);
	bool b = WriteProcessMemory(hProc, (LPVOID)__addr, __originalBuff, __size, NULL);
	CloseHandle(hProc);
}

//ÖØ×ö
void WriteCommand::redo()
{
	HANDLE hProc = ::OpenProcess(PROCESS_ALL_ACCESS, false, __pid);
	bool b = WriteProcessMemory(hProc, (LPVOID)__addr, __newBuff, __size, NULL);
	CloseHandle(hProc);
}