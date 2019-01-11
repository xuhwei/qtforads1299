#ifndef TCPCONNECT_H
#define TCPCONNECT_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QDialog>
#include <QQueue>
#include <QFile>
#include <QVector>



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
     short flag;
     short flagcount;
     short setupNewFile;
     short index_buffer;


private slots:
     void client_tcpConnectSuccess();
     void client_tcpConnectFailed();
     void client_tcpFindHost();
     void client_tcpReadData();

     void server_newConnectFromClient();
     void server_tcpReadData();
};

#endif // TCPCONNECT_H
