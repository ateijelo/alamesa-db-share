#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <csignal>
#include <cstdio>

#include <QDir>
#include <QUuid>
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

static int service_publisher_pid = 0;

void cleanup() {
    printf("cleanup; service publisher pid = %d\n", service_publisher_pid);
    if (service_publisher_pid != 0) {
        kill(service_publisher_pid, SIGKILL);
    }
}

void handle_signal(int signal) {
    Q_UNUSED(signal);
    cleanup();
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), server(qApp), logs(this)
{
    ui->setupUi(this);
    //QString dbfile = findDatabaseFile();

    //logs = new LogsDialog(this);

    QSettings settings;
    QString dbfile = settings.value("dbfile", "").toString();

    connect(ui->actionLogs, &QAction::triggered, this, &MainWindow::showLogs);

    ui->imgWidget->addOffset(-0.03, 0);

    setAcceptDrops(true);

    if (dbfile.isEmpty()) {
        ui->msgLabel->setText("Drag a database file here");
    } else {
        startServer(dbfile);
    }

    std::atexit(cleanup);
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);
    std::signal(SIGABRT, handle_signal);
}

MainWindow::~MainWindow()
{
    service_publisher_pid = 0;
    servicePublisher.kill();
    servicePublisher.waitForFinished(1000);
    delete ui;
}

void MainWindow::startServer(QString dbfile)
{

    if (servicePublisher.state() == QProcess::Running) {
        service_publisher_pid = 0;
        servicePublisher.kill();
        servicePublisher.waitForFinished(5000);
    }

    if (server.isListening()) {
        server.stopListening();
    }

    QFileInfo fi(dbfile);
    date = fi.baseName().right(8);
    hostname = "hostname-not-implemented";

    QString program;
    QStringList args;
    QProcess hostname_cmd;
#ifdef Q_OS_MAC
    program = "scutil";
    args << "--get" << "LocalHostName";
#endif
#ifdef Q_OS_LINUX
    program = "hostname";
#endif
    if (!program.isEmpty()) {
        hostname_cmd.start(program, args);
        hostname_cmd.waitForFinished(500);
        QByteArray ba = hostname_cmd.readAll();
        hostname = QString::fromLatin1(ba).trimmed() + ".local";
    }

    port = 4040;
    while (port < 4140) {
        server.listen(
            QHostAddress::Any, port,
            [this, dbfile](QHttpRequest *req, QHttpResponse *resp) {

                auto path = req->url().path();
                QString logline = QString("GET %1 %2").arg(path).arg(req->remoteAddress());

                if (path == "/ios/last.txt")
                {
                    logline += " 200";
                    reply_iOS_LastTxt(resp);
                }
                else if (path == QString("/alamesa-%1.db").arg(date))
                {
                    auto ft = new FileTransfer(dbfile, req, resp, this);
                    logline += QString(" 200 (id %1)").arg(ft->id());
                    connect(ft, &FileTransfer::transferred, this, &MainWindow::fileTransferred);
                    //new FileTransfer("/etc/shadow", req, resp, this);
                }
                else if (path == "/logs") {
                    if (logs.filename().isEmpty()) {
                        resp->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
                        resp->addHeader("Content-type", "text/html");
                        resp->end("<html><head><title>Not Found</title></head><body><h1>404 - Not Found</h1></body>\n");
                    } else {
                        logs.flush();
                        resp->addHeader("Content-type", "text/plain");
                        new FileTransfer(logs.filename(), req, resp, this);
                    }
                }
                else
                {
                    logline += " 404";
                    resp->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
                    resp->addHeader("Content-type", "text/html");
                    resp->end("<html><head><title>Not Found</title></head><body><h1>404 - Not Found</h1></body>\n");
                }
                logs.writeLine(logline);
            }
        );
        if (server.isListening())
            break;
        port++;
    }

    //setWindowFlags(windowFlags() |= Qt::FramelessWindowHint);

    setWindowTitle("AlaMesa DB Share");
    setWindowIcon(QIcon(":/icon.png"));


    if (server.isListening()) {
        args.clear();
        program.clear();
        qDebug() << "server is listening on port" << port;
//        qDebug() << "publishing.";

#ifdef Q_OS_MAC
        program = "dns-sd";
        args << "-R" << "AlaMesa DB Updates" << "_alamesaupdates._tcp" << "local" << QString("%1").arg(port);
#endif
#ifdef Q_OS_LINUX
        program = "avahi-publish-service";
        args << "AlaMesa DB Updates" << "_alamesaupdates._tcp" << QString("%1").arg(port);
#endif
        if (!program.isEmpty()) {
            servicePublisher.start(program, args);
            service_publisher_pid = servicePublisher.pid();
            ui->msgLabel->setText(QString("Sharing database: <b>%1.db</b><br>(on <i>%2:%3</i>)")
                                  .arg(fi.baseName())
                                  .arg(hostname)
                                  .arg(port));
        } else {
            ui->msgLabel->setText("Service publishing is not implemented");
        }
    } else {
        qDebug() << "server failed to listen";
        ui->msgLabel->setText("Server failed to listen");
    }
}

void MainWindow::showLogs()
{
    logs.show();
}

void MainWindow::fileTransferred(qint64 bytes, qint64 filesize)
{
    QString id = "";
    FileTransfer* ft = qobject_cast<FileTransfer*>(sender());
    if (ft != nullptr) {
        id = ft->id();
    }
    logs.writeLine(QString("transferred %1 bytes out of %2 (id %3)").arg(bytes).arg(filesize).arg(id));
    logs.addTransfer(qreal(bytes) / filesize);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    delta = event->globalPos() - pos();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    move(event->globalPos() - delta);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
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

void MainWindow::dropEvent(QDropEvent *event)
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

void MainWindow::reply_iOS_LastTxt(QHttpResponse *resp)
{
    QByteArray dburl = QString("%1|http://%2:%3/alamesa-%4.db")
                        .arg(date)
                        .arg(hostname)
                        .arg(port)
                        .arg(date)
                        .toLatin1();
    resp->setStatusCode(qhttp::ESTATUS_OK);
    resp->addHeader("Cache-Control", "no-cache");
//    qDebug() << "sending" << dburl;
    resp->write(dburl);
    resp->end("\n");
}

QString MainWindow::findDatabaseFile()
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

void MainWindow::loadDatabaseFromURL(const QUrl &url)
{
    QString dbfile = url.path();
    QSettings settings;
    settings.setValue("dbfile", dbfile);
    startServer(dbfile);
}


