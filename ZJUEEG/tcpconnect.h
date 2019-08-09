#ifndef TCPCONNECT_H
#define TCPCONNECT_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QDialog>
#include <QQueue>
#include <QFile>
#include <QVector>
#include <QLabel>

//特别注意：为修改方便，接受逻辑适配32通道，不适配其他通道数情况。非32通道通读该代码后请自行修改相关参数与逻辑

using namespace std;

enum TcpType{
    Tcp_server,
    Tcp_client,
};

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
     QString setGlazerOnOff(bool);

     QTcpSocket *newconnection;
     QTcpSocket *client;
     QTcpServer *server;

     QQueue<qint8> mark;
     QQueue<double> data_from_wifi;
     QQueue<bool> elect_lead_off;

     bool startBoard;
     bool commandReturn;

signals:
     void boardStart();
private:
     QLabel* command_return_label;
     QFile *datafile;
     QFile *glazerfile;
     TcpType server_client;
     QString ip;
     quint16 port;
     QString storePath;

     QByteArray wifiBuffer;
     bool hasSetupNewFile;
     bool glazer_on;
     int channel_number;
     int packet_size;

private slots:
     void client_tcpConnectSuccess();
     void client_tcpConnectFailed();
     void client_tcpFindHost();
     void client_tcpReadData();

     void server_newConnectFromClient();
     void server_tcpReadData();
};

#endif // TCPCONNECT_H
