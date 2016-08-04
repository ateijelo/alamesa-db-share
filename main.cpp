#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    a.setOrganizationDomain("alamesacuba.com");
    a.setOrganizationName("AlaMesa");
    a.setApplicationDisplayName("AlaMesa DB Share");
    a.setApplicationName("AlaMesa DB Share");
    a.setApplicationVersion("1.0");

    MainWindow w;
    w.setWindowTitle("AlaMesa DB Share");
    w.show();

    return a.exec();
}
