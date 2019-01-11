#-------------------------------------------------
#
# Project created by QtCreator 2018-12-08T17:50:08
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ZJUEEG
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    myframe.cpp \
    signalprocess.cpp \
    tcpconnect.cpp

HEADERS  += mainwindow.h \
    myframe.h \
    signalprocess.h \
    tcpconnect.h

FORMS    += mainwindow.ui
