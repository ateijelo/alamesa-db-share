#ifndef DIALOG_H
#define DIALOG_H

#include <QMainWindow>
#include <QProcess>
#include <qhttpserver.hpp>

namespace Ui {
class MainWindow;
}

#include "logsdialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    ~MainWindow();

    void startServer(QString dbfile);

protected slots:
    void showLogs();
    void fileTransferred(qint64 bytes, qint64 filesize);

private:
    QString findDatabaseFile();
    void loadDatabaseFromURL(const QUrl& url);
    void reply_iOS_LastTxt(qhttp::server::QHttpResponse *resp);

    Ui::MainWindow *ui;
    qhttp::server::QHttpServer server;
    QPoint delta;
    QProcess servicePublisher;
    QString date;
    QString hostname;
    LogsDialog logs;
    int port;
};

#endif // DIALOG_H
