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
    channel_number(N_channel),
    command_return_label(label),
    ip(ip_in),
    port(port_in)
{
    connect_type = _no_connect;
    hasSetupNewFile = false;
    startBoard = false;
    glazer_on = false;
    commandReturn = false;
    com_find_head = false;

    if(channel_number < 32){
        packet_size = 8+4*channel_number +8;
    }
    else{
        packet_size = 8+(4*32+8)*(channel_number/32);//包大小
    }


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

    if(new_thread != NULL)
        new_thread->exit(0);
        delete new_thread;
    if(serial_com != NULL)
        delete serial_com;
}

void TcpConnect::updatePort(quint16 port_in){
    port = port_in;
}
void TcpConnect::updateIp(QString ip_in){
    ip = ip_in;
}
//运行过程中通道不可更改。
//按下停止键后数据已从队列中pop出，这里不用再进行处理
//需要新建文件用于存储
void TcpConnect::setChArg(int ch_number){
    channel_number = ch_number;
    if(channel_number < 32){
        packet_size = 8+4*channel_number +8;
    }
    else{
        packet_size = 8+(4*32+8)*(channel_number/32);//包大小
    }
    datafile->close();
    hasSetupNewFile = false;
}
//客户端开始连接
void TcpConnect::client_connectToBoard(){
    client->abort();
    client->connectToHost(QHostAddress(ip),port);
}
//服务端开始连接
void TcpConnect::server_connectToBoard(){
    startBoard = false;
    server->listen(QHostAddress::Any, port);
}



/*************** 客户端槽函数 ************
 *
 */
//SLOT 连接成功
void TcpConnect::client_tcpConnectSuccess(){
    QMessageBox::information(this,"Message","connect to board success,IP = " + ip + " port = " + QString::number(port,10));
    connect_type = _wifi;
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
            return;
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
            if(channel_number<32){
                for(int channel = 0 ; channel<channel_number; ++channel){
                     QByteArray _4byte =  wifiBuffer.left(4);//取4个字节
                     wifiBuffer.remove(0,4);//从buffer中删去读取的4个字节
                     int perchannel_data = (_4byte[0]&0x000000FF)|((_4byte[1]&0x000000FF)<<8)|((_4byte[2]&0x000000FF)<<16)|((_4byte[3]&0x000000FF)<<24);
                     double r4 = ((double)perchannel_data)/24.0;//24倍增益
                     data_from_wifi.append(r4);//将QBytearray转换成double存入队列
                }
                //读取8字节电极脱落数据,每字节8bit ,含8通道 p或n状态
                //elect_lead_off[64] 为：ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n
                QByteArray _8byte =  wifiBuffer.left(8);
                wifiBuffer.remove(0,8);
                for (int i=0; i<channel_number*2; ++i){
                    elect_lead_off.append(_8byte[i/8] & (1<<i));
                }
            }
            else{
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

void TcpConnect::flush(){
     wifiBuffer.clear();
     mark.clear();
     data_from_wifi.clear();
     elect_lead_off.clear();
}
void TcpConnect::sendToBoard(QByteArray data){
    if(connect_type == _wifi){
        newconnection->write(data);
    }
    else if(connect_type == _serial_com){
        serial_com->serial_port->write(data);
    }
}

void TcpConnect::start_com(QSerialPort *port){
    connect_type = _serial_com;
    //单向传输，此时就设置为板子启动
    startBoard = true;
    emit boardStart();
    QMessageBox::information(this,"Connecting", "串口连接成功！",QMessageBox::Yes);

    if(serial_com != NULL){
        delete serial_com;
        serial_com = NULL;
    }
    if(new_thread != NULL){
        new_thread->exit(0);
        new_thread->wait();
        delete new_thread;
        new_thread = NULL;
    }
    serial_com = new com_commnicate(port, this);
    new_thread = new QThread();
    connect(new_thread,SIGNAL(started()),serial_com,SLOT(com_prepare()));
    serial_com->moveToThread(new_thread);
    new_thread->start();
}
void TcpConnect::stop_com(){
    com_find_head = false;
    flush();
}
void com_commnicate::com_prepare(){
    connect(serial_port, SIGNAL(readyRead()), this, SLOT(com_run()));
}
void com_commnicate::com_run(){
    if(!tcp_obj->startBoard){
        qDebug()<<"forbid running here";
        /*
        QString data = serial_port->readAll();
        if (data == QString(tr("NMCTemp"))){
            QMessageBox::information(this,"Connecting", "server receive : " + data + "   ->连接成功！",QMessageBox::Yes);
            tcp_obj->startBoard = true;
            emit tcp_obj->boardStart();
        }
        else{
            qDebug()<<"receive not NMCTemp: "<<data;
        }
        */
    }
    else{
        //调试模式，接收发送命令的返回值
        if(tcp_obj->commandReturn){
            //memset(com_buffer, 0, MAX_SIZE_COM_BUFFER*sizeof(char));
            //ReadFile(*com_handle,com_buffer,1,&wCount,NULL);
            QByteArray tmpdata = serial_port->readAll();
            int return_data = int(tmpdata[0]&0xFF);
            QString content = QString::number(return_data,16);
            tcp_obj->command_return_label->setText(content);
        }

        //接收逻辑
        //注：没有设置读取buffer大小，默认为无限制，适用于数据不能丢失场合。dataready信号在BUffer中有数据待读取和新接收到数据时均会触发。但会合并触发。
        //memset(com_buffer, 0, MAX_SIZE_COM_BUFFER*sizeof(char));
        //ReadFile(*com_handle,com_buffer,MAX_SIZE_COM_BUFFER,&wCount,NULL);
        QByteArray tmpdata = serial_port->readAll();

        //新建文件用于存储原始数据
        if(!tcp_obj->hasSetupNewFile){
            delete tcp_obj->datafile;
            QString current_time = QDateTime::currentDateTime().toString("yyyy-MM-dd[hh-mm-ss]");
            tcp_obj->datafile = new QFile(tcp_obj->storePath+"/"+ current_time + ".txt");
            if(!tcp_obj->datafile->open(QIODevice::WriteOnly)){
                QMessageBox::warning(this,"warning", "can't open file to save data",QMessageBox::Yes);
            }
            else
                tcp_obj->hasSetupNewFile = true;
        }
        tcp_obj->datafile->write(tmpdata);
        if(tcp_obj->glazer_on){
            tcp_obj->glazerfile->write(tmpdata);
        }
        //**********************************   数据包解析 ****************************************************
        //************* 数据包包含4个部分：包序号（4字节，每字节小端先发）+mark（4字节，mark位于最小端最小bit位上）+通道数据（32*4字节，每字节小端先发）+脱落数据（8字节，展开为ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n）
        //*************
        tcp_obj->wifiBuffer.append(tmpdata);
        if(!tcp_obj->com_find_head){
            //寻找包起始位置
            int _packet_size = tcp_obj->packet_size+1;
            if(tcp_obj->wifiBuffer.size()<_packet_size*3){
                return;
            }
            int move_count=0;
            while (move_count<_packet_size+1){
                QByteArray _nbyte =  tcp_obj->wifiBuffer.left(_packet_size+4);//取1个包加4字节数据
                unsigned int ch_number1 = (_nbyte[0]&0xFF)|((_nbyte[1]&0xFF)<<8)|((_nbyte[2]&0xFF)<<16)|((_nbyte[3]&0xFF)<<24);
                unsigned int ch_number2 = (_nbyte[_packet_size]&0xFF)|((_nbyte[_packet_size+1]&0xFF)<<8)|((_nbyte[_packet_size+2]&0xFF)<<16)|((_nbyte[_packet_size+3]&0xFF)<<24);
                if(ch_number1+1 == ch_number2){
                    packet_number_last = ch_number1;
                    tcp_obj->wifiBuffer.remove(0,_packet_size);
                    break;
                }
                tcp_obj->wifiBuffer.remove(0,1);
                move_count++;
            }
            if(move_count >= (_packet_size+1)){
                qDebug()<<"Error: can not find data packet head";
                return;
            }
            tcp_obj->com_find_head = true;
        }
        while(tcp_obj->wifiBuffer.size()>=tcp_obj->packet_size+1){//包长度：4+4+4*通道数+8=144。由于Tcp传输中自行拆包为不同长短的包进行传输，这里等待足够一包的数据量后进行处理
            //包序号检测
            QByteArray packet_head_4_byte =  tcp_obj->wifiBuffer.left(4);
            unsigned int packet_number_now = (packet_head_4_byte[0]&0xFF)|((packet_head_4_byte[1]&0xFF)<<8)|((packet_head_4_byte[2]&0xFF)<<16)|((packet_head_4_byte[3]&0xFF)<<24);;
            if(packet_number_last+1!=packet_number_now){
                tcp_obj->com_find_head = false;
                return;
            }
            packet_number_last = packet_number_now;
            tcp_obj->wifiBuffer.remove(0,4);
            //移除前3个字节冗余
            tcp_obj->wifiBuffer.remove(0,3);
            //获取一个字节，目前仅8bit中的最低位代表标记信息，1有效，其他位均为0，即不做处理
            bool ok;
            qint8 mark_tmp = tcp_obj->wifiBuffer.left(1).toHex().toInt(&ok,16);
            tcp_obj->wifiBuffer.remove(0,1);
            tcp_obj->mark.append(mark_tmp);
            if(tcp_obj->channel_number<32){
                for(int channel = 0 ; channel<tcp_obj->channel_number; ++channel){
                     QByteArray _4byte =  tcp_obj->wifiBuffer.left(4);//取4个字节
                     tcp_obj->wifiBuffer.remove(0,4);//从buffer中删去读取的4个字节
                     int perchannel_data = (_4byte[0]&0x000000FF)|((_4byte[1]&0x000000FF)<<8)|((_4byte[2]&0x000000FF)<<16)|((_4byte[3]&0x000000FF)<<24);
                     double r4 = ((double)perchannel_data)/24.0;//24倍增益
                     tcp_obj->data_from_wifi.append(r4);//将QBytearray转换成double存入队列
                }
                //读取8字节电极脱落数据,每字节8bit ,含8通道 p或n状态
                //elect_lead_off[64] 为：ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n
                QByteArray _8byte =  tcp_obj->wifiBuffer.left(8);
                tcp_obj->wifiBuffer.remove(0,8);
                for (int i=0; i<tcp_obj->channel_number*2; ++i){
                    tcp_obj->elect_lead_off.append(_8byte[i/8] & (1<<i));
                }
            }
            else{
                int times_32channel = tcp_obj->channel_number/32;
                for(int channel32count = 0; channel32count<times_32channel; ++channel32count){
                    //填充通道数据
                    for(int channel = 0 ; channel<32; ++channel){
                         QByteArray _4byte =  tcp_obj->wifiBuffer.left(4);//取4个字节
                         tcp_obj->wifiBuffer.remove(0,4);//从buffer中删去读取的4个字节
                         int perchannel_data = (_4byte[0]&0x000000FF)|((_4byte[1]&0x000000FF)<<8)|((_4byte[2]&0x000000FF)<<16)|((_4byte[3]&0x000000FF)<<24);
                         double r4 = ((double)perchannel_data)/24.0;//24倍增益
                         tcp_obj->data_from_wifi.append(r4);//将QBytearray转换成double存入队列
                    }
                    //读取8字节电极脱落数据,每字节8bit ,含8通道 p或n状态
                    //elect_lead_off[64] 为：ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n
                    for (int i = 0; i<8; ++i){
                        bool ok;
                        qint8 _1byte = tcp_obj->wifiBuffer.left(1).toHex().toInt(&ok,16);
                        tcp_obj->wifiBuffer.remove(0,1);
                        if(!ok){
                            qDebug("can not convert lead off data");
                        }
                        else{
                            tcp_obj->elect_lead_off.append(_1byte&0x01);
                            tcp_obj->elect_lead_off.append(_1byte&0x02);
                            tcp_obj->elect_lead_off.append(_1byte&0x04);
                            tcp_obj->elect_lead_off.append(_1byte&0x08);
                            tcp_obj->elect_lead_off.append(_1byte&0x10);
                            tcp_obj->elect_lead_off.append(_1byte&0x20);
                            tcp_obj->elect_lead_off.append(_1byte&0x40);
                            tcp_obj->elect_lead_off.append(_1byte&0x80);
                         }
                    }
                }
            }
            //移除累加和校验
            tcp_obj->wifiBuffer.remove(0,1);
        }
        //*************
        //***************************      数据包解析结束        ********************
        if(tcp_obj->datafile->size()>100000000){
            tcp_obj->datafile->close();
            tcp_obj->hasSetupNewFile = false;
        }
    }
}
