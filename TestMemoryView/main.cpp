#include "TestMemoryView.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
	QApplication::setStyle("Fusion");
    TestMemoryView w;
    w.show();
    return a.exec();
}
