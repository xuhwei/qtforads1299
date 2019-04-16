#ifndef TCPCONNECT_H
#define TCPCONNECT_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QDialog>
#include <QQueue>
#include <QFile>
#include <QVector>

//特别注意：为修改方便，接受逻辑适配32通道，不适配其他通道数情况。非32通道通读该代码后请自行修改相关参数与逻辑

using namespace std;

enum TcpType{
    Tcp_server,
    Tcp_client,
};

class TcpConnect:QDialog
{    
    Q_OBJECT
public:
     explicit TcpConnect( QString ip_in, quint16 port_in,TcpType tcp_type,QWidget *parent = 0);
     ~TcpConnect();

     void client_connectToBoard();
     void server_connectToBoard();
     void setStartBoard(bool start);
     void setStorePath(QString path);

     QTcpSocket *newconnection;
     QTcpSocket *client;
     QTcpServer *server;

     QQueue<qint8> mark;
     QQueue<double> data_from_wifi;
     QQueue<bool> elect_lead_off;

     bool startBoard;     

private:
     QFile *datafile;
     TcpType server_client;
     QString ip;
     quint16 port;
     QString storePath;

     QByteArray wifiBuffer;
     double sampleRate;
     short setupNewFile;
     double ch_data_last[32];//用于丢包时构建数据
     unsigned int packet_number_last;//存储上一个包序号//无符号整型避免编号大于 0x7F FF FF FF时判为负数
     bool falg_rev_firstpacket;//标识是否收到第一个数据包

private slots:
     void client_tcpConnectSuccess();
     void client_tcpConnectFailed();
     void client_tcpFindHost();
     void client_tcpReadData();

     void server_newConnectFromClient();
     void server_tcpReadData();
};

#endif // TCPCONNECT_H
