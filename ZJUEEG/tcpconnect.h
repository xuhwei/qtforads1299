#ifndef TCPCONNECT_H
#define TCPCONNECT_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QDialog>
#include <QQueue>
#include <QFile>
#include <QVector>
#include <QLabel>
#include <QObject>
#include <QThread>
#include <windows.h>
#include <QSerialPort>

#define MAX_SIZE_COM_BUFFER 192

using namespace std;

enum TcpType{
    Tcp_server,
    Tcp_client,
};

enum ConnectType{
    _serial_com,
    _wifi,
    _no_connect
};
class com_commnicate;

class TcpConnect:public QDialog
{    
    Q_OBJECT
public:
     explicit TcpConnect( QString ip_in, quint16 port_in,TcpType tcp_type, double N_channel, QLabel* label);
     ~TcpConnect();

     void client_connectToBoard();
     void server_connectToBoard();
     void setStorePath(QString );
     void updatePort(quint16);
     void updateIp(QString);
     void setChArg(int ch_number);
     void flush();
     void sendToBoard(QByteArray);
     void start_com(QSerialPort* port);
     void stop_com();
     QString setGlazerOnOff(bool);

     QTcpSocket *newconnection;
     QTcpSocket *client;
     QTcpServer *server;

     QQueue<qint8> mark;
     QQueue<double> data_from_wifi;
     QQueue<bool> elect_lead_off;

     bool startBoard;
     bool commandReturn;  
     bool hasSetupNewFile;
     bool glazer_on;
     bool com_find_head;
     int channel_number;
     int packet_size;
     QByteArray wifiBuffer;
     QLabel* command_return_label;
     QFile *datafile;
     QFile *glazerfile;
     QString storePath;
     ConnectType connect_type;

signals:
     void boardStart();

private:    
     TcpType server_client;
     QString ip;
     quint16 port;
     QThread *new_thread = NULL;
     com_commnicate *serial_com = NULL;

private slots:
     void client_tcpConnectSuccess();
     void client_tcpConnectFailed();
     void client_tcpFindHost();
     void client_tcpReadData();

     void server_newConnectFromClient();
     void server_tcpReadData();
};

class com_commnicate: public QDialog
{
    Q_OBJECT
public:
    com_commnicate(QSerialPort *port, TcpConnect *obj){
        serial_port = port;
        tcp_obj = obj;
        keeprun = true;
        memset(com_buffer,0,MAX_SIZE_COM_BUFFER*sizeof(char));
        packet_number_last = 0;
    }
    //void com_run();
    QSerialPort *serial_port;
public slots:
    void com_run();
    void com_prepare();    
private:    
    TcpConnect *tcp_obj;

    unsigned int packet_number_last;
    bool keeprun;
    char com_buffer[MAX_SIZE_COM_BUFFER];
    DWORD wCount;
};

#endif // TCPCONNECT_H
