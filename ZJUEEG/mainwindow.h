#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QCloseEvent>
#include <QTime>
#include <QLabel>
#include <QAbstractNativeEventFilter>
#include <windows.h>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "connect.h"
#include "signalprocess.h"
#include "glazer.h"
#include "qcustomplot/qcustomplot.h"
#include "channelset.h"
#include "fftset.h"
#include "stft.h"
#include "portset.h"

class myframe;
class QCheckBox;
class TimeCount;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


    QVector<QVector<double>> data;
    QVector<bool> elect_off_p;
    QVector<bool> elect_off_n;
    QVector<qint8> mark;
    QVector<QString> vector_port_name;
    QSerialPort *serial_port = nullptr;

protected:
    void closeEvent( QCloseEvent * event);
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);

private slots:
    void stop_m();
    void run_m();

    void openChannelSetWidget();
    void openFFTSetWidget();
    void openSTFTWidget();
    void openConnectSetWidget();
    void openPortSetWidget();
    void openDebugWidget();
    void changePwmFreq();
    void openfile();
    void connectWifi();
    void connectToBardDone();
    void getPort();
    void getIp();
    void changeChannelNumber();
    void changeSampleRate();
    void changeNotchFilter();
    void changeBandFilter();
    void changeHpassFilter();
    void changeFhFilter();
    void changeFlFilter();
    void changeAmplititude();
    void changeTimeScale();
    void changeChannelEnable();
    void changeSavePath();
    void changeSingleBipolarElect();
    void changeRMS();
    void changeCommandReturn();
    void sendCommand();
    void showhelpmessage();
    void glazerStart();
    void glazerEnd();
    void glazerResult();
    void initFFTWidget(bool,double,double,double,double);
    void portSetDone();
    void updateIniFile();

private:

    void initVectorDrawFrame();
    void initVectorQCheckBox();
    void drawFFT();
    void flush();
    void initCom();
    void scanPort();

    Ui::MainWindow *ui;

    QString readFilePath;
    quint16 port;
    QString ip;
    bool notchFilter;
    bool bandFilter;
    bool hpassFilter;
    bool rms_enable;
    int rms_w_count;//rms 窗计数
    int rms_w_length;//rms 窗长
    double fl_bandPass;
    double fh_bandPass;
    int channel_number;
    int per_channel_number;
    double sampleRate;  
    bool running;
    bool runFileData;
    bool singleElectMode;
    bool fft_selfControl;
    double fft_xmin,fft_xmax,fft_ymin,fft_ymax;
    bool find_com;
    QPen pen[32];
    QVector<bool>  channel_enable_map;
    CommnicateBoard *tcp;
    SignalProcess *dataProcess;
    QVector<myframe *>  drawframe;
    QVector<QCheckBox *> checkbox;
    QFile *readFile;
    QString glazerfilename;

    Glazer* glazer_window;
    TimeCount* glazertimer;
    STFT* stft_window = nullptr;

    //qint32 port_bandrate;

};
//计时功能，继承QThread来实现多线程。
class TimeCount:public QThread
{
     Q_OBJECT
public:
    TimeCount(QLabel *label){
        p_label = label;
        timepass = 0;
        running = true;
        zerotime = new QTime(0,0,0);
    }
    ~TimeCount(){
        delete zerotime;
    }
    void stopRun(){running =false;}
protected:
    void run(){
        while(running){
            p_label->setText(zerotime->addSecs(timepass).toString("hh:mm:ss"));
            sleep(1);
            timepass ++;
        }
    }
private:
    unsigned int timepass;
    bool running;
    QLabel *p_label;
    QTime *zerotime;
};
#endif // MAINWINDOW_H
