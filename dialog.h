#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QProcess>
#include <qhttpserver.hpp>

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    ~Dialog();

    void startServer(QString dbfile);

protected slots:

private:
    QString findDatabaseFile();
    void loadDatabaseFromURL(const QUrl& url);
    void reply_iOS_LastTxt(qhttp::server::QHttpResponse *resp);

    Ui::Dialog *ui;
    qhttp::server::QHttpServer server;
    QPoint delta;
    QProcess servicePublisher;
    QString date;
    QString hostname;
    int port;
};

#endif // DIALOG_H
