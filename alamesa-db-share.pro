#-------------------------------------------------
#
# Project created by QtCreator 2016-07-25T10:21:58
#
#-------------------------------------------------

QT       += core gui network
CONFIG   += c++11 c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = "AlaMesa DB Share"
TEMPLATE = app
ICON = icon.icns

INCLUDEPATH += qhttp


SOURCES += main.cpp\
        qhttp/http-parser/http_parser.c \
        qhttp/qhttpabstracts.cpp \
        qhttp/qhttpserver.cpp \
        qhttp/qhttpserverconnection.cpp \
        qhttp/qhttpserverrequest.cpp \
        qhttp/qhttpserverresponse.cpp \
    filetransfer.cpp \
    imagewidget.cpp \
    mainwindow.cpp \
    logsdialog.cpp

HEADERS  += \
        qhttp/private/httpparser.hxx \
        qhttp/private/httpreader.hxx \
        qhttp/private/httpwriter.hxx \
        qhttp/private/qhttpbase.hpp \
        qhttp/private/qhttpclient_private.hpp \
        qhttp/private/qhttpclientrequest_private.hpp \
        qhttp/private/qhttpclientresponse_private.hpp \
        qhttp/private/qhttpserver_private.hpp \
        qhttp/private/qhttpserverconnection_private.hpp \
        qhttp/private/qhttpserverrequest_private.hpp \
        qhttp/private/qhttpserverresponse_private.hpp \
        qhttp/private/qsocket.hpp \
        qhttp/http-parser/http_parser.h \
        qhttp/qhttpabstracts.hpp \
        qhttp/qhttpfwd.hpp \
        qhttp/qhttpserver.hpp \
        qhttp/qhttpserverconnection.hpp \
        qhttp/qhttpserverrequest.hpp \
        qhttp/qhttpserverresponse.hpp \
    filetransfer.h \
    imagewidget.h \
    mainwindow.h \
    logsdialog.h

FORMS    += \
    mainwindow.ui \
    logsdialog.ui

RESOURCES += \
    resources.qrc
