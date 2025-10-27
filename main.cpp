#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QApplication::setWindowIcon(QIcon(":/images/app.ico"));

    MainWindow window;
    window.show();

    return QApplication::exec();
}