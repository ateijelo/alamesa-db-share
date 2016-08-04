#ifndef FILETRANSFER_H
#define FILETRANSFER_H

#include <QFile>
#include <QUuid>
#include <QLinkedList>
#include <boost/icl/interval_set.hpp>

#include "qhttpserverresponse.hpp"

class FileTransfer : public QObject {
    Q_OBJECT
public:
    FileTransfer(const QString& filename,
                 qhttp::server::QHttpRequest *req,
                 qhttp::server::QHttpResponse *resp, QObject *parent = 0);
    ~FileTransfer();
    QString id();

signals:
    void transferred(qint64, qint64);

private:
    typedef boost::icl::interval_set<qint64>::type ranges;

    void serve();
    ranges parseRanges(const QByteArray& rangeHeader, qint64 filesize);
    void responseDone(bool wasTheLastPacket);
    void responseDestroyed();

    char *buffer;
    qint64 buffer_size;
    qint64 transfer_size = 0;
    QFile file;
    ranges file_ranges;
//    static QRegularExpression rangeHeaderRE;
//    qint64 file_pos;
    qhttp::server::QHttpResponse *resp;
    QLinkedList<QByteArray> boundaries;
    bool now_goes_a_boundary = false;
    void sendBoundary();
    void finish();
    QUuid _id;
};

#endif // FILETRANSFER_H
