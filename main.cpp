#include "copyfinder.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CopyFinder w;
    w.show();

    return a.exec();
}
