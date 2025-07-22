#include "mywindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    myWindow myw;
    myw.show();
    return a.exec();
}
