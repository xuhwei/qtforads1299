#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
#include <QCloseEvent>

#include "tcpconnect.h"
#include "signalprocess.h"

class myframe;
class QCheckBox;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    QVector<QVector<double>> data;
    QVector<bool> elect_off_p;
    QVector<bool> elect_off_n;
    QVector<qint8> mark;

protected:
    void closeEvent( QCloseEvent * event);

private slots:
    void stop_m();
    void run_m();
    void changeSavePath();
    void changeNotchFilter();
    void changeBandFilter();
    void changeHpassFilter();
    void connectWifi();
    void changeFhFilter();
    void changeFlFilter();
    void getPort();
    void getIp();
    void changeAmplititude();
    void changeTimeScale();
    void changeChannelEnable();
    void sendCommand();
    void openfile();

private:

    void initVectorDrawFrame();
    void initVectorQCheckBox();

    Ui::MainWindow *ui;

    QString saveFilePath;
    QString readFilePath;
    quint16 port;
    QString ip;
    bool notchFilter;
    bool bandFilter;
    double fl_bandPass;
    double fh_bandPass;
    int time_scale;
    int ampli_scale;
    int channel_number;
    int per_channel_number;
    double sampleRate;
    QVector<bool>  channel_enable_map;
    bool running;
    bool runFileData;

    TcpConnect *tcp;
    SignalProcess *dataProcess;
    QVector<myframe *>  drawframe;
    QVector<QCheckBox *> checkbox;
    QFile *readFile;
};

#endif // MAINWINDOW_H
