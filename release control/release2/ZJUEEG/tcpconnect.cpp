#include "tcpconnect.h"
#include <QMessageBox>
#include <QtWidgets>
#include <QtGui>
#include <QDateTime>


TcpConnect::TcpConnect( QString ip_in, quint16 port_in,TcpType tcp_type, QWidget *parent):QDialog(parent){
    ip = ip_in;
    port = port_in;
    falg_rev_firstpacket = false;
    packet_number_last = 0;

    if (Tcp_server == tcp_type){
        server_client = Tcp_server;
        server = new QTcpServer();

        connect(server,SIGNAL(newConnection()),this,SLOT(server_newConnectFromClient()));
    }
    else if(Tcp_client == tcp_type){
        server_client = Tcp_client;
        client = new QTcpSocket();

        connect(client,SIGNAL(connected()),this,SLOT(client_tcpConnectSuccess()));
        connect(client,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(client_tcpConnectFailed()));
        connect(client,SIGNAL(readyRead()),this,SLOT(client_tcpReadData()));
        //connect(client,SIGNAL(hostFound()),this,SLOT(client_tcpFindHost()));
    }
    else
        qDebug("tcpTypeError");


    setupNewFile = 0;
    sampleRate =1000.0;
    setupNewFile = 0;
    startBoard = false;
    storePath = "";
    datafile = new QFile();
}

TcpConnect::~TcpConnect(){
    if (Tcp_server == server_client)
        delete server;
    else if (Tcp_client == server_client)
        delete client;
}



//客户端开始连接
void TcpConnect::client_connectToBoard(){
    client->abort();
    client->connectToHost(QHostAddress(ip),port);
}
//服务端开始连接
void TcpConnect::server_connectToBoard(){
    server->listen(QHostAddress::Any, port);
}



/*************** 客户端槽函数 ************
 *
 *
 */
//SLOT 连接成功
void TcpConnect::client_tcpConnectSuccess(){
    QMessageBox tip;
    tip.setText("connect to board success,IP = " + ip + " port = " + QString::number(port,10));
    tip.exec();
}
//SLOT 连接失败
void TcpConnect::client_tcpConnectFailed(){
    QMessageBox tip;
    QString errormessage = "tcp connect failed: " + client->errorString();
    tip.setText(errormessage);
    tip.exec();
}
//SLOT 发现主机
void TcpConnect::client_tcpFindHost(){
    //QMessageBox tip;
    //tip.setText("client find host");
    //tip.exec();
    qDebug("client find host");
}
//SLOT 接收到数据
void TcpConnect::client_tcpReadData(){

}



/***********   服务端槽函数  **************
 *
 *
*/

void TcpConnect::server_newConnectFromClient(){
    //QMessageBox tip;
    //tip.setText("get connect requestion of client  ");
    //tip.exec();
    qDebug("has got new socket");
    //获得新的SOCKET
    newconnection = server->nextPendingConnection();
    QObject::connect(newconnection,SIGNAL(readyRead()),this,SLOT(server_tcpReadData()));
}

void TcpConnect::server_tcpReadData(){
    if(!startBoard){
        QString data = newconnection->readAll();
        if (data == QString(tr("NMCTemp"))){
            //QMessageBox tip;
            //tip.setText("server receive : " + data + "   ->连接成功！");
            //tip.exec();
            QMessageBox::information(this,"Connecting", "server receive : " + data + "   ->连接成功！",QMessageBox::Yes);
            setStartBoard(true);
        }
        else{
            //QMessageBox tip;
            //tip.setText("error:server receive : " + data);
            //tip.exec();
            qDebug("receive not NMCTemp");
        }
    }
    else{
        //qDebug("hasdata");
        //接收逻辑
        //注：没有设置读取buffer大小，默认为无限制，适用于数据不能丢失场合。dataready信号在BUffer中有数据待读取和新接收到数据时均会触发。但会合并触发。
        QByteArray tmpdata = newconnection->readAll();

        //新建文件用于存储原始数据
        if(!setupNewFile){
            delete datafile;
            QString current_time = QDateTime::currentDateTime().toString("yyyy-MM-dd[hh-mm-ss]");
            datafile = new QFile(storePath+"/"+ current_time + ".txt");
            if(!datafile->open(QIODevice::WriteOnly)){
                QMessageBox::warning(this,"warning", "can't open file to save data",QMessageBox::Yes);
            }
            else
                setupNewFile = 1;
        }
        datafile->write(tmpdata);

        //**********************************   数据包解析 ****************************************************
        //************* 数据包包含4个部分：包序号（4字节，每字节小端先发）+mark（4字节，mark位于最小端最小bit位上）+通道数据（32*4字节，每字节小端先发）+脱落数据（8字节，展开为ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n）
        //*************
        wifiBuffer.append(tmpdata);       
        while(wifiBuffer.size()>=144){//包长度：4+4+4*32+8=144。由于Tcp传输中自行拆包为不同长短的包进行传输，这里等待足够一包的数据量后进行处理
            //移除前4个字节序号标记
            wifiBuffer.remove(0,4);
            //移除前3个字节冗余
            wifiBuffer.remove(0,3);
            //获取一个字节，目前仅8bit中的最低位代表标记信息，1有效，其他位均为0，即不做处理
            bool ok;
            qint8 mark_tmp = wifiBuffer.left(1).toHex().toInt(&ok,16);
            wifiBuffer.remove(0,1);
            mark.append(mark_tmp);
            //填充32通道数据
            for(int channel = 0 ; channel<32; ++channel){
                 QByteArray _4byte =  wifiBuffer.left(4);//取4个字节
                 wifiBuffer.remove(0,4);//从buffer中删去读取的4个字节
                 int perchannel_data = (_4byte[0]&0x000000FF)|((_4byte[1]&0x000000FF)<<8)|((_4byte[2]&0x000000FF)<<16)|((_4byte[3]&0x000000FF)<<24);
                 double r4 = ((double)perchannel_data)/24.0;//24倍增益
                 data_from_wifi.append(r4);//将QBytearray转换成double存入队列
            }
            //读取8字节电极脱落数据,每字节8bit ,含8通道 p或n状态
            //elect_lead_off[64] 为：ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n
            for (int i = 0; i<8; ++i){
                bool ok;
                qint8 _1byte = wifiBuffer.left(1).toHex().toInt(&ok,16);
                wifiBuffer.remove(0,1);
                if(!ok){
                    qDebug("can not convert lead off data");
                }
                else{
                    elect_lead_off.append(_1byte&0x01);
                    elect_lead_off.append(_1byte&0x02);
                    elect_lead_off.append(_1byte&0x04);
                    elect_lead_off.append(_1byte&0x08);
                    elect_lead_off.append(_1byte&0x10);
                    elect_lead_off.append(_1byte&0x20);
                    elect_lead_off.append(_1byte&0x40);
                    elect_lead_off.append(_1byte&0x80);
                 }
            }
        }
        //*************
        //***************************      数据包解析结束        ********************
        if(datafile->size()>100000000){
            datafile->close();
            setupNewFile = 0;
        }
    }
}


void TcpConnect::setStartBoard(bool start){
    startBoard = start;
}

void TcpConnect::setStorePath(QString path){
    storePath = path;
}
