#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
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
    ~Dialog();

private:
    Ui::Dialog *ui;
    qhttp::server::QHttpServer server;
    QPoint delta;
};

#endif // DIALOG_H
