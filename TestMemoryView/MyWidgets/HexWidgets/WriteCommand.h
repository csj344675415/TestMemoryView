#pragma once

#include <QUndoCommand>

class WriteCommand : public QUndoCommand
{
public:
	WriteCommand(unsigned int pid, unsigned long long addr, unsigned char* buf1, unsigned char* buf2, unsigned int size, QUndoCommand *parent = nullptr);
	~WriteCommand();

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

