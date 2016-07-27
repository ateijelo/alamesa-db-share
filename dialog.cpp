#include "dialog.h"
#include "ui_dialog.h"

#include <QFile>
#include <qhttpserver.hpp>
#include <qhttpserverrequest.hpp>
#include <qhttpserverresponse.hpp>

#include "filetransfer.h"

using namespace qhttp::server;

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog), server(qApp)
{
    server.listen(
        QHostAddress::Any, 4040,
        [this](QHttpRequest *req, QHttpResponse *resp){
            if (req->url().path() == "/ios/last.txt") {
                resp->setStatusCode(qhttp::ESTATUS_OK);
                resp->end("Hello, world!");
            } else if (req->url().path() == "/file") {
                new FileTransfer("/home/andy/posfile", req, resp, this);
                //new FileTransfer("/media/andy/mirror/Xcode_7.3.dmg", req, resp, this);
            } else {
                resp->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
                resp->addHeader("Content-type", "text/html");
                resp->end("<html><head><title>Not Found</title></head><body><h1>404 - Not Found</h1></body>\n");
            }
        }
    );

    setWindowTitle("AlaMesa DB Share");

    if (server.isListening()) {

    } else {
        qDebug() << "server failed to listen";
    }

    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}
