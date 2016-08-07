#ifndef LOGSDIALOG_H
#define LOGSDIALOG_H

#include <QDialog>
#include <QFile>
#include <QLinkedList>

namespace Ui {
class LogsDialog;
}

class LogsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LogsDialog(QWidget *parent = 0);
    ~LogsDialog();
    void addTransfer(qreal r);
    QString filename();
    void flush();
    void writeLine(const QString& msg);

private:
    Ui::LogsDialog *ui;
    QFile logfile;
//    QLinkedList<QString> lines;
//    int maxlines = 10000;
    qreal transfers = 0;
};

#endif // LOGSDIALOG_H
