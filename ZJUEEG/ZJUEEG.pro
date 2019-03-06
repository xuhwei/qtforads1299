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
    tcpconnect.cpp \
    Biquad.cpp \
    Butterworth.cpp \
    Cascade.cpp \
    ChebyshevI.cpp \
    ChebyshevII.cpp \
    Custom.cpp \
    draw.cpp \
    PoleFilter.cpp \
    RBJ.cpp \
    State.cpp

HEADERS  += mainwindow.h \
    myframe.h \
    signalprocess.h \
    tcpconnect.h \
    Iir.h \
    Biquad.h \
    Butterworth.h \
    Cascade.h \
    ChebyshevI.h \
    ChebyshevII.h \
    Common.h \
    Custom.h \
    draw.h \
    Layout.h \
    MathSupplement.h \
    PoleFilter.h \
    RBJ.h \
    State.h \
    Types.h

FORMS    += mainwindow.ui
