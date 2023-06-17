#pragma once

#include <QUndoCommand>

#ifndef STRUCT_MemoryViewCommand
typedef struct COMMADN_PARAMETER
{
	unsigned long long addr;
	unsigned int pid;
	unsigned int size;
	unsigned char* originalBuff;
	unsigned char* newBuff;
};
#define STRUCT_MemoryViewCommand
#endif

class MemoryViewCommand : public QUndoCommand
{
public:
	MemoryViewCommand(COMMADN_PARAMETER cp, QUndoCommand *parent = nullptr);
	~MemoryViewCommand();

	void undo() override;	//³·Ïú
	void redo() override;	//ÖØ×ö

private:
	unsigned int __pid;
	unsigned long long __addr;
	unsigned int __size;
	unsigned char __originalBuff[256];
	unsigned char __newBuff[256];
	//unsigned char* __originalBuff;
	//unsigned char* __newBuff;
};


