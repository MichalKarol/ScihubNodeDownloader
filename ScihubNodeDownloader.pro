#-------------------------------------------------
#
# Project created by QtCreator 2016-09-17T15:52:38
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ScihubNodeDownloader
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    searchtab.cpp \
    resultstab.cpp \
    producttab.cpp \
    authdialog.cpp

HEADERS  += widget.h \
    searchtab.h \
    resultstab.h \
    producttab.h \
    authdialog.h \
    product.h

DISTFILES += \
    README.md \
    LICENSE
