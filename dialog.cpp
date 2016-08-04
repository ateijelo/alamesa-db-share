#include "dialog.h"
#include "ui_dialog.h"

#include <QDir>
#include <QFile>
#include <QMimeData>
#include <QMouseEvent>
#include <QNetworkInterface>
#include <QSettings>
#include <qhttpserver.hpp>
#include <qhttpserverrequest.hpp>
#include <qhttpserverresponse.hpp>

#include "filetransfer.h"

using namespace qhttp::server;

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog), server(qApp)
{
    //QString dbfile = findDatabaseFile();

    QSettings settings;
    QString dbfile = settings.value("dbfile", "").toString();

    ui->setupUi(this);
    ui->imgWidget->addOffset(-0.03, 0);

    setAcceptDrops(true);

    if (dbfile.isEmpty()) {
        ui->msgLabel->setText("Drag a database file here");
    } else {
        startServer(dbfile);
    }
}

Dialog::~Dialog()
{
    servicePublisher.kill();
    servicePublisher.waitForFinished(1000);
    delete ui;
}

void Dialog::startServer(QString dbfile)
{

    if (servicePublisher.state() == QProcess::Running) {
        servicePublisher.kill();
        servicePublisher.waitForFinished(5000);
    }

    if (server.isListening()) {
        server.stopListening();
    }

    QFileInfo fi(dbfile);
    date = fi.baseName().right(8);
    hostname = "hostname-only-works-in-osx";

    QString program;
    QStringList args;
#ifdef Q_OS_MAC
    QProcess sc;
    program = "scutil";
    args << "--get" << "LocalHostName";
    sc.start(program, args);
    sc.waitForFinished(500);
    QByteArray ba = sc.readAll();
    hostname = QString::fromLatin1(ba).trimmed() + ".local";
#endif

    port = 4040;
    while (port < 4140) {
        server.listen(
            QHostAddress::Any, port,
            [this, dbfile](QHttpRequest *req, QHttpResponse *resp){
                if (req->url().path() == "/ios/last.txt") {
                    reply_iOS_LastTxt(resp);
                } else if (req->url().path() == QString("/alamesa-%1.db").arg(date)) {
                    new FileTransfer(dbfile, req, resp, this);
                    //new FileTransfer("/etc/shadow", req, resp, this);
                } else {
                    resp->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
                    resp->addHeader("Content-type", "text/html");
                    resp->end("<html><head><title>Not Found</title></head><body><h1>404 - Not Found</h1></body>\n");
                }
            }
        );
        if (server.isListening())
            break;
        port++;
    }

    //setWindowFlags(windowFlags() |= Qt::FramelessWindowHint);

    setWindowTitle("AlaMesa DB Share");


    if (server.isListening()) {
        args.clear();

        qDebug() << "server is listening on port" << port;
        qDebug() << "publishing.";

#ifdef Q_OS_MAC
        program = "dns-sd";
        args << "-R" << "AlaMesa DB Updates" << "_alamesaupdates._tcp" << "local" << QString("%1").arg(port);
        servicePublisher.start(program, args);
        ui->msgLabel->setText(QString("Sharing database: <b>%1.db</b>").arg(fi.baseName()));
#endif

    } else {
        qDebug() << "server failed to listen";
        ui->msgLabel->setText("Server failed to listen");
    }
}

void Dialog::mousePressEvent(QMouseEvent *event)
{
    delta = event->globalPos() - pos();
}

void Dialog::mouseMoveEvent(QMouseEvent *event)
{
    move(event->globalPos() - delta);
}

void Dialog::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        QUrl u = event->mimeData()->urls().first();
        QRegExp rx(".*/alamesa.*[0-9]{8}.db");
        //rx.setPatternSyntax(QRegExp::Wildcard);
        if (rx.exactMatch(u.path())) {
            event->acceptProposedAction();
        }
    }
}

void Dialog::dropEvent(QDropEvent *event)
{
    QUrl dburl;
    for (auto& url: event->mimeData()->urls()) {
        dburl = url;
        break;
    }
    event->acceptProposedAction();
    if (dburl.isValid())
        loadDatabaseFromURL(dburl);
}

void Dialog::reply_iOS_LastTxt(QHttpResponse *resp)
{
    QByteArray dburl = QString("%1|http://%2:%3/alamesa-%4.db")
                        .arg(date)
                        .arg(hostname)
                        .arg(port)
                        .arg(date)
                        .toLatin1();
    resp->setStatusCode(qhttp::ESTATUS_OK);
    resp->addHeader("Cache-Control", "no-cache");
    qDebug() << "sending" << dburl;
    resp->write(dburl);
    resp->end("\n");
}

QString Dialog::findDatabaseFile()
{
    QDir appdir(qApp->applicationDirPath());
    appdir.cdUp();
    appdir.cdUp();
    appdir.cdUp();

    QStringList filters;
    filters << "alamesa-????????.db";

    for (QString s: appdir.entryList(filters, QDir::Files, QDir::Time)) {
        QString dbfile = appdir.absoluteFilePath(s);
        qDebug() << "Using db file:" << dbfile;
        return dbfile;
    }
    return QString();
}

void Dialog::loadDatabaseFromURL(const QUrl &url)
{
    QString dbfile = url.path();
    QSettings settings;
    settings.setValue("dbfile", dbfile);
    qDebug() << "url.path():" << dbfile;
    startServer(dbfile);
}
