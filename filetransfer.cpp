#include <QtDebug>
#include <QUuid>
#include <iostream>

#include <QRegularExpression>
#include <qhttpserverrequest.hpp>

#include "filetransfer.h"

using namespace qhttp::server;

using namespace boost::icl;
using std::cout;
using std::endl;

static QRegularExpression rangeHeaderRE(
        R"(^\s*bytes\s*=\s*(?:\d*\s*-\s*\d*)(?:\s*,\s*\d*\s*-\s*\d*)*\s*$)",
        QRegularExpression::OptimizeOnFirstUsageOption
);

FileTransfer::FileTransfer(const QString& filename, QHttpRequest *req, QHttpResponse *resp, QObject *parent)
    : QObject(parent), file(filename), resp(resp)
{
    _id = QUuid::createUuid();

    buffer_size = 64*1024;
    buffer = new char[buffer_size];

    connect(resp, &QHttpResponse::done, this, &FileTransfer::responseDone);
    connect(resp, &QObject::destroyed, this, &FileTransfer::responseDestroyed);

    if (!file.exists()) {
        resp->setStatusCode(qhttp::ESTATUS_NOT_FOUND);
        resp->end();
        return;
    }

    if (!file.open(QFile::ReadOnly)) {
        resp->setStatusCode(qhttp::ESTATUS_FORBIDDEN);
        resp->end();
        return;
    }

    if (req->headers().has("range")) {
        file_ranges = parseRanges(req->headers().value("range"), file.size());
    }

    resp->setStatusCode(qhttp::ESTATUS_OK);
    if (!resp->headers().has("content-type"))
        resp->addHeader("Content-type", "application/octet-stream");
    resp->addHeader("Accept-Ranges", "bytes");

    if (file_ranges.iterative_size() == 0) {
        resp->addHeader("Content-Length",QString("%1").arg(file.size()).toLatin1());
        file_ranges.add(discrete_interval<qint64>::right_open(0,file.size()));
    } else if (file_ranges.iterative_size() == 1) {
        qint64 a = first(file_ranges);
        qint64 b = last(file_ranges);

        if (a < 0) { // my convention in parseRanges for Not Satisfiable
            resp->setStatusCode(qhttp::ESTATUS_REQUESTED_RANGE_NOT_SATISFIABLE);
            resp->end();
            return;
        } else {
            resp->setStatusCode(qhttp::ESTATUS_PARTIAL_CONTENT);
            resp->addHeader(
                "Content-Range",
                 QString("bytes %1-%2/%3").arg(a).arg(b).arg(file.size()).toLatin1()
            );
            resp->addHeader("Content-Length", QString("%1").arg(file_ranges.size()).toLatin1());
        }
    } else if (file_ranges.iterative_size() > 1) {
        resp->setStatusCode(qhttp::ESTATUS_PARTIAL_CONTENT);
        now_goes_a_boundary = true;

//        char s[17]; s[16] = '\0';
//        for (int i = 0; i < 16; i++) {
//            s[i] = "0123456789abcdef"[qrand()%16];
//        }
        QString boundary(_id.toString());
        boundary.remove(QRegularExpression("[{}-]"));
        qint64 content_length = 0;
        for (auto it = file_ranges.begin(); it != file_ranges.end(); it++) {
            auto i = *it;
            qint64 from = first(i);
            qint64 to = last(i);
            QByteArray b = QString(
                                "\r\n--%1\r\n"
                                "Content-type: application/octet-stream\r\n"
                                "Content-range: bytes %2-%3/%4\r\n"
                                "\r\n"
                             )
                             .arg(boundary).arg(from).arg(to).arg(file.size())
                             .toLatin1();
            boundaries.append(b);
            content_length += b.length();
            content_length += to - from + 1;
        }
        QByteArray last = QString("\r\n--%1--\r\n").arg(boundary).toLatin1();
        content_length += last.length();
        boundaries.append(last);

        resp->addHeader(
            "Content-type",
            QString("multipart/byteranges; boundary=%1").arg(boundary).toLatin1()
        );
        resp->addHeader("Content-length", QString("%1").arg(content_length).toLatin1());
    }

//    cout << "transfering range: " << file_ranges << endl;

//    auto a = discrete_interval<int>::closed(0,0);
//    auto b = discrete_interval<int>::closed(1,1);
//    interval_set<int> s;
//    s.add(a).add(b);
//    s.add(discrete_interval<int>::closed(10,20));

//    cout << "s.size(): " << s.size() << endl;
//    cout << "s.iterative_size(): " << s.iterative_size() << endl;

//    cout << s << endl;

//    for (interval_set<int>::iterator i = s.begin(); i != s.end(); i++) {
//        auto j = *i;
//        cout << first(j) << " - " << last(j) << endl;
//    }

//    resp->setStatusCode(qhttp::ESTATUS_OK);
//    resp->addHeader("Content-type", "application/octet-stream");
//    resp->addHeader("Content-Length",QString("%1").arg(file.size()).toLatin1());
//    resp->addHeader("Accept-Ranges", "bytes");

    if (req->method() == qhttp::EHTTP_HEAD) {
        resp->end();
        return;
    }

    connect(resp, &QHttpResponse::allBytesWritten, this, &FileTransfer::serve);
    serve();
}

FileTransfer::~FileTransfer()
{
    free(buffer);
    emit transferred(transfer_size, file.size());
//    qDebug() << "Transfer" << (void*)this << "deleted";
}

QString FileTransfer::id()
{
    return _id.toString().remove(QRegularExpression("[{}]"));
}

static QRegularExpression rangeSpecRE(R"((?<start>\d*)\s*-\s*(?<end>\d*))");

FileTransfer::ranges FileTransfer::parseRanges(
        const QByteArray &rangeHeader, qint64 filesize)
{
    QString s = QString::fromLatin1(rangeHeader);

    ranges r;
    ranges emptyrange;
    ranges not_satisfiable;
    not_satisfiable.add(discrete_interval<qint64>::closed(-1,-1));

    if (!rangeHeaderRE.match(s).hasMatch()) {
        qDebug() << "returning empty range";
        return emptyrange;
    }

    auto it = rangeSpecRE.globalMatch(s);
    while (it.hasNext()) {
        auto m = it.next();

        QString start = m.captured("start");
        QString end = m.captured("end");

        if (start.isEmpty() && end.isEmpty())
            continue;

        bool a_ok;
        qint64 a = start.toLongLong(&a_ok);
        if (a >= filesize) {
            return not_satisfiable;
        }

        bool b_ok;
        qint64 b = end.toLongLong(&b_ok);

        if (start.isEmpty()) {
            if (!b_ok) return emptyrange;
            r.add(discrete_interval<qint64>::right_open(filesize - b, filesize));
        } else if (end.isEmpty()) {
            if (!a_ok) return emptyrange;
            r.add(discrete_interval<qint64>::right_open(a, filesize));
        } else {
            if (!a_ok || !b_ok) return emptyrange;
            r.add(discrete_interval<qint64>::closed(a,b));
        }
    }
    return r;
}

void FileTransfer::responseDone(bool wasTheLastPacket)
{
    Q_UNUSED(wasTheLastPacket);
//    qDebug() << "responseDone" << endl;
    deleteLater();
}

void FileTransfer::responseDestroyed()
{
//    qDebug() << "responseDestroyed" << endl;    
    deleteLater();
}

void FileTransfer::sendBoundary()
{
    if (!boundaries.empty()) {
        QByteArray b = boundaries.takeFirst();
        resp->write(b);
        now_goes_a_boundary = false;
    }
}

void FileTransfer::finish()
{
    sendBoundary();
    resp->end();
    deleteLater();
}

void FileTransfer::serve()
{
    //qint64 from = file.pos();
//    file.seek(file_pos);

    if (file_ranges.iterative_size() == 0) {
        finish();
        return;
    }

    auto i = *(file_ranges.begin());
    qint64 from = first(i);
    qint64 to = last(i) + 1;

    file.seek(from);
    qint64 toread = std::min(buffer_size, to - from);
    qint64 bytes_read = file.read(buffer, toread);
    if (bytes_read <= 0) {
        finish();
        return;
    }

    //qDebug() << "Transfer" << (void*)this << "sending from" << from << "to" << (from + r);
//    file_pos += r;

    sendBoundary();
    transfer_size += bytes_read;
    resp->write(QByteArray::fromRawData(buffer, bytes_read));

    size_t n = file_ranges.iterative_size();
    file_ranges.subtract(discrete_interval<qint64>::right_open(from, from + bytes_read));
    size_t m = file_ranges.iterative_size();
    if (n != m) {
        // one less subinterval
        now_goes_a_boundary = true;
        // in the next read/write op the boundary will be sent before the data
    }
}
