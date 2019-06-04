/*
 * 用于Tcp连接，包含数据接收，解析，存储
 * 包数据格式规定：一包144字节。
 *          包序号（4字节，每字节小端先发）+mark（4字节，mark位于最小端最小bit位上）+通道数据（32*4字节，每字节小端先发）+脱落数据（8字节，展开为ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n）
 * 默认存储路径：当前路径/ZJUEEGDATA
 * 默认接收Buffer大小：无限制
 * 默认作为服务器
 * 默认端口61613（从mainwindow接收）
 *
 * 对外提供接收到的数据接口：
     QQueue<qint8> mark;
     QQueue<double> data_from_wifi;
     QQueue<bool> elect_lead_off;
     硬件启动标识：bool startBoard
*/

#include "tcpconnect.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDir>

TcpConnect::TcpConnect( QString ip_in, quint16 port_in,TcpType tcp_type,double N_channel,QLabel * label):
    command_return_label(label),
    ip(ip_in),
    port(port_in),
    channel_number(N_channel)
{
    hasSetupNewFile = false;
    startBoard = false;
    glazer_on = false;
    commandReturn = false;
    packet_size = 8+(4*32+8)*(channel_number/32);//包大小

    //确定TCP连接类型
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
    //新建文件夹，确定存储路径
    QString path = QDir::currentPath();
    storePath = path+"/ZJUEEGDATA";
    QDir dir(storePath);
    if(!dir.exists()){
        bool ok =dir.mkdir(storePath);
        if(!ok)
            qDebug("save path mkdir failed");
    }
    datafile = new QFile();
}

TcpConnect::~TcpConnect(){
    if (Tcp_server == server_client)
        delete server;
    else if (Tcp_client == server_client)
        delete client;
}

void TcpConnect::updatePort(quint16 port_in){
    port = port_in;
}
void TcpConnect::updateIp(QString ip_in){
    ip = ip_in;
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
 */
//SLOT 连接成功
void TcpConnect::client_tcpConnectSuccess(){
    QMessageBox::information(this,"Message","connect to board success,IP = " + ip + " port = " + QString::number(port,10));
    //QMessageBox tip;
    //tip.setText("connect to board success,IP = " + ip + " port = " + QString::number(port,10));
    //tip.exec();
}
//SLOT 连接失败
void TcpConnect::client_tcpConnectFailed(){
    QMessageBox::information(this,"Message","tcp connect failed: " + client->errorString());
    //QMessageBox tip;
    //QString errormessage = "tcp connect failed: " + client->errorString();
    //tip.setText(errormessage);
    //tip.exec();
}
//SLOT 发现主机
void TcpConnect::client_tcpFindHost(){
    qDebug("client find host");
}
//SLOT 接收到数据
void TcpConnect::client_tcpReadData(){

}



/***********   服务端槽函数  **************
 *
*/
void TcpConnect::server_newConnectFromClient(){
    qDebug("has got new socket");
    //获得新的SOCKET
    newconnection = server->nextPendingConnection();
    QObject::connect(newconnection,SIGNAL(readyRead()),this,SLOT(server_tcpReadData()));
}

void TcpConnect::server_tcpReadData(){
    if(!startBoard){
        QString data = newconnection->readAll();
        if (data == QString(tr("NMCTemp"))){
            QMessageBox::information(this,"Connecting", "server receive : " + data + "   ->连接成功！",QMessageBox::Yes);
            startBoard = true;
            emit boardStart();
        }
        else{
            qDebug("receive not NMCTemp");
        }
    }
    else{
        //调试模式，接收发送命令的返回值
        if(commandReturn){
            QByteArray tmpdata = newconnection->readAll();
            int return_data = int(tmpdata[0]&0xFF);
            QString content = QString::number(return_data,16);
            command_return_label->setText(content);
        }

        //接收逻辑
        //注：没有设置读取buffer大小，默认为无限制，适用于数据不能丢失场合。dataready信号在BUffer中有数据待读取和新接收到数据时均会触发。但会合并触发。
        QByteArray tmpdata = newconnection->readAll();

        //新建文件用于存储原始数据
        if(!hasSetupNewFile){
            delete datafile;
            QString current_time = QDateTime::currentDateTime().toString("yyyy-MM-dd[hh-mm-ss]");
            datafile = new QFile(storePath+"/"+ current_time + ".txt");
            if(!datafile->open(QIODevice::WriteOnly)){
                QMessageBox::warning(this,"warning", "can't open file to save data",QMessageBox::Yes);
            }
            else
                hasSetupNewFile = true;
        }
        datafile->write(tmpdata);
        if(glazer_on){
            glazerfile->write(tmpdata);
        }

        //**********************************   数据包解析 ****************************************************
        //************* 数据包包含4个部分：包序号（4字节，每字节小端先发）+mark（4字节，mark位于最小端最小bit位上）+通道数据（32*4字节，每字节小端先发）+脱落数据（8字节，展开为ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n）
        //*************
        wifiBuffer.append(tmpdata);       
        while(wifiBuffer.size()>=packet_size){//包长度：4+4+4*通道数+8=144。由于Tcp传输中自行拆包为不同长短的包进行传输，这里等待足够一包的数据量后进行处理
            //移除前4个字节序号标记
            wifiBuffer.remove(0,4);
            //移除前3个字节冗余
            wifiBuffer.remove(0,3);
            //获取一个字节，目前仅8bit中的最低位代表标记信息，1有效，其他位均为0，即不做处理
            bool ok;
            qint8 mark_tmp = wifiBuffer.left(1).toHex().toInt(&ok,16);
            wifiBuffer.remove(0,1);
            mark.append(mark_tmp);
            int times_32channel = channel_number/32;
            for(int channel32count = 0; channel32count<times_32channel; ++channel32count){
                //填充通道数据
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
        }
        //*************
        //***************************      数据包解析结束        ********************
        if(datafile->size()>100000000){
            datafile->close();
            hasSetupNewFile = false;
        }
    }
}

void TcpConnect::setStorePath(QString path){
    storePath = path;
}

QString TcpConnect::setGlazerOnOff(bool on){
    QString filename;
    glazer_on = on;
    if(on){
        QString current_time = QDateTime::currentDateTime().toString("yyyy-MM-dd[hh-mm-ss]");
        filename = storePath+"/"+ current_time + "-glazer.txt";
        glazerfile = new QFile(filename);
        if(!glazerfile->open(QIODevice::WriteOnly)){
            QMessageBox::warning(this,"warning", "can't open file to save glazer data",QMessageBox::Yes);
        }
    }
    else{
        glazerfile->close();
        delete glazerfile;
    }
    return filename;
}
