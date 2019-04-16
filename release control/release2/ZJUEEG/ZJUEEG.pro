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
    State.cpp \
    envolope/envolope.cpp \
    envolope/envolope_data.cpp \
    envolope/envolope_initialize.cpp \
    envolope/envolope_terminate.cpp \
    envolope/fft.cpp \
    envolope/fft1.cpp \
    envolope/ifft.cpp \
    envolope/rt_nonfinite.cpp \
    envolope/rtGetInf.cpp \
    envolope/rtGetNaN.cpp

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
    Types.h \
    envolope/envolope.h \
    envolope/envolope_data.h \
    envolope/envolope_initialize.h \
    envolope/envolope_terminate.h \
    envolope/envolope_types.h \
    envolope/fft.h \
    envolope/fft1.h \
    envolope/ifft.h \
    envolope/rt_nonfinite.h \
    envolope/rtGetInf.h \
    envolope/rtGetNaN.h \
    envolope/rtwtypes.h

FORMS    += mainwindow.ui

DISTFILES +=

RESOURCES += \
    zjueeg.qrc
