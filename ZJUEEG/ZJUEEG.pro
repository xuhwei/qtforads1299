#-------------------------------------------------
#
# Project created by QtCreator 2018-12-08T17:50:08
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ZJUEEG
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    myframe.cpp \
    signalprocess.cpp \
    tcpconnect.cpp \
    glazer.cpp \
    qcustomplot/qcustomplot.cpp \
    iir/Biquad.cpp \
    iir/Butterworth.cpp \
    iir/Cascade.cpp \
    iir/ChebyshevI.cpp \
    iir/ChebyshevII.cpp \
    iir/Custom.cpp \
    iir/PoleFilter.cpp \
    iir/RBJ.cpp \
    iir/State.cpp \
    FFT/FFT.cpp \
    FFT/fft2.cpp \
    FFT/FFT_emxAPI.cpp \
    FFT/FFT_emxutil.cpp \
    FFT/FFT_initialize.cpp \
    FFT/FFT_terminate.cpp \
    FFT/rt_nonfinite.cpp \
    FFT/rtGetInf.cpp \
    FFT/rtGetNaN.cpp \
    channelset.cpp \
    stft.cpp \
    fftset.cpp


HEADERS  += mainwindow.h \
    myframe.h \
    signalprocess.h \
    tcpconnect.h \
    Iir.h \
    glazer.h \
    qcustomplot/qcustomplot.h \
    iir/Biquad.h \
    iir/Butterworth.h \
    iir/Cascade.h \
    iir/ChebyshevI.h \
    iir/ChebyshevII.h \
    iir/Common.h \
    iir/Custom.h \
    iir/Layout.h \
    iir/MathSupplement.h \
    iir/PoleFilter.h \
    iir/RBJ.h \
    iir/State.h \
    iir/Types.h \
    FFT/FFT.h \
    FFT/fft2.h \
    FFT/FFT_emxAPI.h \
    FFT/FFT_emxutil.h \
    FFT/FFT_initialize.h \
    FFT/FFT_terminate.h \
    FFT/FFT_types.h \
    FFT/rt_nonfinite.h \
    FFT/rtGetInf.h \
    FFT/rtGetNaN.h \
    FFT/rtwtypes.h \
    channelset.h \
    stft.h \
    sp++/include/advmath-impl.h \
    sp++/include/advmath.h \
    sp++/include/algraph-impl.h \
    sp++/include/algraph.h \
    sp++/include/amgraph-impl.h \
    sp++/include/amgraph.h \
    sp++/include/avltree-impl.h \
    sp++/include/avltree.h \
    sp++/include/bfgs-impl.h \
    sp++/include/bfgs.h \
    sp++/include/binaryheap-impl.h \
    sp++/include/binaryheap.h \
    sp++/include/bstree-impl.h \
    sp++/include/bstree.h \
    sp++/include/bwt-impl.h \
    sp++/include/bwt.h \
    sp++/include/ccholesky-impl.h \
    sp++/include/ccholesky.h \
    sp++/include/cevd-impl.h \
    sp++/include/cevd.h \
    sp++/include/cholesky-impl.h \
    sp++/include/cholesky.h \
    sp++/include/classicalpse-impl.h \
    sp++/include/classicalpse.h \
    sp++/include/conjgrad-impl.h \
    sp++/include/conjgrad.h \
    sp++/include/constants.h \
    sp++/include/convolution-impl.h \
    sp++/include/convolution.h \
    sp++/include/convolution_usefftw-impl.h \
    sp++/include/convolution_usefftw.h \
    sp++/include/correlation-impl.h \
    sp++/include/correlation.h \
    sp++/include/correlation_usefftw-impl.h \
    sp++/include/correlation_usefftw.h \
    sp++/include/cqrd-impl.h \
    sp++/include/cqrd.h \
    sp++/include/csvd-impl.h \
    sp++/include/csvd.h \
    sp++/include/cwt-impl.h \
    sp++/include/cwt.h \
    sp++/include/cwt_usefftw-impl.h \
    sp++/include/cwt_usefftw.h \
    sp++/include/dfd-impl.h \
    sp++/include/dfd.h \
    sp++/include/dgt-impl.h \
    sp++/include/dgt.h \
    sp++/include/dgt_usefftw-impl.h \
    sp++/include/dgt_usefftw.h \
    sp++/include/doublelist-impl.h \
    sp++/include/doublelist.h \
    sp++/include/dwt-impl.h \
    sp++/include/dwt.h \
    sp++/include/eigenanalysispse-impl.h \
    sp++/include/eigenanalysispse.h \
    sp++/include/evd-impl.h \
    sp++/include/evd.h \
    sp++/include/fastconv-impl.h \
    sp++/include/fastconv.h \
    sp++/include/fft-impl.h \
    sp++/include/fft.h \
    sp++/include/fftmr-impl.h \
    sp++/include/fftmr.h \
    sp++/include/fftpf-impl.h \
    sp++/include/fftpf.h \
    sp++/include/fftr2-impl.h \
    sp++/include/fftr2.h \
    sp++/include/fftw-impl.h \
    sp++/include/fftw.h \
    sp++/include/fftw3.h \
    sp++/include/filtercoefs-impl.h \
    sp++/include/filtercoefs.h \
    sp++/include/fir-impl.h \
    sp++/include/fir.h \
    sp++/include/fitcurves.h \
    sp++/include/funcroot-impl.h \
    sp++/include/funcroot.h \
    sp++/include/function.h \
    sp++/include/functions.h \
    sp++/include/hashtable-impl.h \
    sp++/include/hashtable.h \
    sp++/include/huffmancode-impl.h \
    sp++/include/huffmancode.h \
    sp++/include/huffmancode2-impl.h \
    sp++/include/huffmancode2.h \
    sp++/include/iir-impl.h \
    sp++/include/iir.h \
    sp++/include/integral-impl.h \
    sp++/include/integral.h \
    sp++/include/integrand.h \
    sp++/include/interpolation.h \
    sp++/include/inverse-impl.h \
    sp++/include/inverse.h \
    sp++/include/kalman-impl.h \
    sp++/include/kalman.h \
    sp++/include/levinson-impl.h \
    sp++/include/levinson.h \
    sp++/include/linequs1-impl.h \
    sp++/include/linequs1.h \
    sp++/include/linequs2-impl.h \
    sp++/include/linequs2.h \
    sp++/include/linequs3-impl.h \
    sp++/include/linequs3.h \
    sp++/include/linesearch-impl.h \
    sp++/include/linesearch.h \
    sp++/include/lms-impl.h \
    sp++/include/lms.h \
    sp++/include/lsfitting-impl.h \
    sp++/include/lsfitting.h \
    sp++/include/lud-impl.h \
    sp++/include/lud.h \
    sp++/include/matrix-impl.h \
    sp++/include/matrix.h \
    sp++/include/matrixallocate-impl.h \
    sp++/include/matrixallocate.h \
    sp++/include/matrixinverse-impl.h \
    sp++/include/matrixinverse.h \
    sp++/include/matrixmath-impl.h \
    sp++/include/matrixmath.h \
    sp++/include/newtoninterp-impl.h \
    sp++/include/newtoninterp.h \
    sp++/include/nleroot-impl.h \
    sp++/include/nleroot.h \
    sp++/include/nleroots-impl.h \
    sp++/include/nleroots.h \
    sp++/include/nlfunc.h \
    sp++/include/nlfuncs.h \
    sp++/include/nlfunction.h \
    sp++/include/objfunc.h \
    sp++/include/parametricpse-impl.h \
    sp++/include/parametricpse.h \
    sp++/include/pointerheap-impl.h \
    sp++/include/pointerheap.h \
    sp++/include/pseudoinverse-impl.h \
    sp++/include/pseudoinverse.h \
    sp++/include/qrd-impl.h \
    sp++/include/qrd.h \
    sp++/include/queue-impl.h \
    sp++/include/queue.h \
    sp++/include/random-impl.h \
    sp++/include/random.h \
    sp++/include/rls-impl.h \
    sp++/include/rls.h \
    sp++/include/sort-impl.h \
    sp++/include/sort.h \
    sp++/include/spline3interp-impl.h \
    sp++/include/spline3interp.h \
    sp++/include/stack-impl.h \
    sp++/include/stack.h \
    sp++/include/statistics-impl.h \
    sp++/include/statistics.h \
    sp++/include/steepdesc-impl.h \
    sp++/include/steepdesc.h \
    sp++/include/student.h \
    sp++/include/svd-impl.h \
    sp++/include/svd.h \
    sp++/include/timing-impl.h \
    sp++/include/timing.h \
    sp++/include/toeplitz-impl.h \
    sp++/include/toeplitz.h \
    sp++/include/usingdeclare.h \
    sp++/include/utilities-impl.h \
    sp++/include/utilities.h \
    sp++/include/vector-impl.h \
    sp++/include/vector.h \
    sp++/include/vectormath-impl.h \
    sp++/include/vectormath.h \
    sp++/include/wft-impl.h \
    sp++/include/wft.h \
    sp++/include/wft_usefftw-impl.h \
    sp++/include/wft_usefftw.h \
    sp++/include/wiener-impl.h \
    sp++/include/wiener.h \
    sp++/include/window-impl.h \
    sp++/include/window.h \
    sp++/include/wvd-impl.h \
    sp++/include/wvd.h \
    fftset.h




FORMS    += mainwindow.ui \
    glazer.ui \
    channelset.ui \
    stft.ui \
    fftset.ui

DISTFILES +=

RESOURCES += \
    zjueeg.qrc


