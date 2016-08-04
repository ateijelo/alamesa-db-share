#include <QtDebug>
#include <QStandardPaths>
#include <QDateTime>

#include "logsdialog.h"
#include "ui_logsdialog.h"

LogsDialog::LogsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LogsDialog)
{
    ui->setupUi(this);
    auto docs = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
    if (!docs.empty()) {
        auto filename = docs.first() + "/alamesa-db-share.log";
        logfile.setFileName(filename);
        if (logfile.open(QIODevice::WriteOnly|QIODevice::Append)) {
            ui->logsLabel->setText(filename + ":");
        }
    }
    addTransfer(0);
}

LogsDialog::~LogsDialog()
{
    delete ui;
}

void LogsDialog::addTransfer(qreal r)
{
    transfers += r;
    ui->transfersLabel->setText(QString("<b>Databases transferred</b>: %1").arg(transfers, 0, 'f', 2));
}

QString LogsDialog::filename()
{
    return logfile.fileName();
}

void LogsDialog::flush()
{
    if (logfile.isOpen())
        logfile.flush();
}

void LogsDialog::writeLine(const QString &msg)
{
    QDateTime now = QDateTime::currentDateTime();
    QString now_str = now.toString("yyyy-MM-dd HH:mm:ss.zzz t");
    QString logline = QString("%1: %2").arg(now_str).arg(msg);
    if (logfile.isOpen()) {
        logfile.write(logline.toUtf8() + "\n");
    }
//    lines.push_back(logline);
//    while (lines.size() > maxlines) {
//        lines.pop_front();
//    }
    ui->textEdit->append(logline);
    //ui->textBrowser->scroll();
}
