#include "dialog.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationDisplayName("AlaMesa DB Share");
    a.setApplicationName("AlaMesa DB Share");

    Dialog w;
    w.setWindowTitle("AlaMesa DB Share");
    w.show();

    return a.exec();
}
