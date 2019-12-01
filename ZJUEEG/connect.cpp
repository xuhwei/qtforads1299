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
     硬件启动标识：bool m_has_start_board
*/

#include "connect.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDir>

CommnicateBoard::CommnicateBoard( QString ip_in, quint16 port_in,TcpType tcp_type,int N_channel,QLabel * label):
    p_command_return_label(label),
    m_channel_number(N_channel),
    m_ip(ip_in),
    m_port(port_in)
{
    m_connect_type = _no_connect;
    m_has_setup_newfile = false;
    m_has_start_board = false;
    m_glazer_on = false;
    m_debug_on = false;
    m_command_return_mode = false;
    //m_com_find_head = false;
    m_find_head = false;
    m_packet_number_last = 0;
    //初始计算包大小
    if(m_channel_number < 32){
        m_packet_size = 8+4*m_channel_number +8+1;
    }
    else{
        m_packet_size = 8+(4*32+8)*(m_channel_number/32)+1;//包大小
    }

    //确定TCP连接类型
    if (Tcp_server == tcp_type){
        m_tcp_type = Tcp_server;
        p_tcp_server = new QTcpServer();
        connect(p_tcp_server,SIGNAL(newConnection()),this,SLOT(slot_wifi_tcp_server_new_connect()));
    }
    else if(Tcp_client == tcp_type){
        m_tcp_type = Tcp_client;
        p_tcp_client = new QTcpSocket();
        connect(p_tcp_client,SIGNAL(connected()),this,SLOT(slot_wifi_tcp_client_connect_success()));
        connect(p_tcp_client,SIGNAL(error(QAbstractSocket::SocketError)),this,SLOT(slot_wifi_tcp_client_connect_failed()));
        connect(p_tcp_client,SIGNAL(readyRead()),this,SLOT(slot_wifi_tcp_client_read_data()));
        //connect(client,SIGNAL(hostFound()),this,SLOT(slot_wifi_tcp_client_find_host()));
    }
    else{
        qDebug("tcpTypeError");
    }
    //新建文件夹，确定存储路径
    m_storePath = QDir::currentPath()+"/ZJUEEGDATA";
    QDir dir(m_storePath);
    if(!dir.exists()){
        bool ok =dir.mkdir(m_storePath);
        if(!ok)
            qDebug("save path mkdir failed");
    }
    p_datafile = new QFile();
    p_debugfile = new QFile();
    p_processed_file = new QFile();
}

CommnicateBoard::~CommnicateBoard(){
    if (Tcp_server == m_tcp_type){
        delete p_tcp_server;
        p_tcp_server = nullptr;
    }
    else if (Tcp_client == m_tcp_type){
        delete p_tcp_client;
        p_tcp_client = nullptr;
    }
    if(p_datafile !=nullptr){
        delete p_datafile;
        p_datafile = nullptr;
    }
    if(p_debugfile !=nullptr){
        delete p_debugfile;
        p_debugfile = nullptr;
    }
    if(p_glazerfile != nullptr){
        delete p_glazerfile;
        p_glazerfile = nullptr;
    }
    if(p_processed_file != nullptr){
        delete p_processed_file;
        p_processed_file = nullptr;
    }
    if(p_debug_thread != nullptr){
        delete p_debug_thread;
        p_debug_thread = nullptr;
    }
    //if(p_new_thread != nullptr){
    //    p_new_thread->quit();
    //    p_new_thread->wait();
    //    delete p_new_thread;
    //    p_new_thread = nullptr;
    //}
}

void CommnicateBoard::wifi_update_port(quint16 port_in){
    m_port = port_in;
}
void CommnicateBoard::wifi_update_ip(QString ip_in){
    m_ip = ip_in;
}
//运行过程中通道不可更改。
//按下停止键后数据已从队列中pop出，这里不用再进行处理
//需要新建文件用于存储
void CommnicateBoard::set_channel_Arg(int ch_number){
    m_channel_number = ch_number;
    if(m_channel_number < 32){
        m_packet_size = 8+4*m_channel_number +8+1;
    }
    else{
        m_packet_size = 8+(4*32+8)*(m_channel_number/32)+1;//包大小
    }
    p_datafile->close();
    p_debugfile->close();
    m_has_setup_newfile = false;
}
//客户端开始连接
void CommnicateBoard::wifi_client_connect_board(){
    p_tcp_client->abort();
    p_tcp_client->connectToHost(QHostAddress(m_ip),m_port);
}
//服务端开始连接
void CommnicateBoard::wifi_server_connect_board(){
    m_has_start_board = false;
    p_tcp_server->listen(QHostAddress::Any, m_port);
}



/*************** 客户端槽函数 ************
 *
 */
//SLOT 连接成功
void CommnicateBoard::slot_wifi_tcp_client_connect_success(){
    QMessageBox::information(this,"Message","connect to board success,IP = " + m_ip + " port = " + QString::number(m_port,10));
    m_connect_type = _wifi;
    //QMessageBox tip;
    //tip.setText("connect to board success,IP = " + ip + " port = " + QString::number(port,10));
    //tip.exec();
}
//SLOT 连接失败
void CommnicateBoard::slot_wifi_tcp_client_connect_failed(){
    QMessageBox::information(this,"Message","tcp connect failed: " + p_tcp_client->errorString());
    //QMessageBox tip;
    //QString errormessage = "tcp connect failed: " + client->errorString();
    //tip.setText(errormessage);
    //tip.exec();
}
//SLOT 发现主机
void CommnicateBoard::slot_wifi_tcp_client_find_host(){
    qDebug("client find host");
}
//SLOT 接收到数据
void CommnicateBoard::slot_wifi_tcp_client_read_data(){

}



/***********   服务端槽函数  **************
 *
*/
void CommnicateBoard::slot_wifi_tcp_server_new_connect(){
    qDebug("has got new socket");
    //获得新的SOCKET
    p_tcp_new_connection = p_tcp_server->nextPendingConnection();
    QObject::connect(p_tcp_new_connection,SIGNAL(readyRead()),this,SLOT(slot_read_data()));
    m_connect_type = _wifi;
    m_has_start_board = true;
    emit signal_board_start();
    QMessageBox::information(this,"提示","wifi连接成功！",QMessageBox::Yes);
}

void CommnicateBoard::slot_read_data(){
    //调试模式，接收发送命令的返回值
    if(m_command_return_mode){
        QByteArray tmpdata;
        if(m_connect_type == _wifi){
            tmpdata = p_tcp_new_connection->readAll();
        }
        else if(m_connect_type == _serial_com){
            tmpdata = p_com->readAll();
        }
        else{
            qDebug("error");
            return;
        }
        int return_data = int(tmpdata[0]&0xFF);
        QString content = QString::number(return_data,16);
        p_command_return_label->setText(content);
        return;
    }
    //接收逻辑
    //注：没有设置读取buffer大小，默认为无限制，适用于数据不能丢失场合。dataready信号在BUffer中有数据待读取和新接收到数据时均会触发。但会合并触发。
    QByteArray tmpdata;
    if(m_connect_type == _wifi){
        tmpdata = p_tcp_new_connection->readAll();
    }
    else if(m_connect_type == _serial_com){
        tmpdata = p_com->readAll();
    }
    else{
        qDebug("error");
        return;
    }
    //新建文件用于存储原始数据
    if(!m_has_setup_newfile){
        delete p_datafile;
        QString current_time = QDateTime::currentDateTime().toString("yyyy-MM-dd[hh-mm-ss]");
        //if(m_connect_type == _wifi){
        //    p_datafile = new QFile(m_storePath+"/" + "wifi_" + current_time + ".txt");
        //}
        //else if(m_connect_type == _serial_com){
        //    p_datafile = new QFile(m_storePath+"/"+ "serial_" + current_time + ".txt");
        //}
        //else{
        //    QMessageBox::warning(this,"warning", "Error connect type",QMessageBox::Yes);
        //    return;
        //}
        p_datafile = new QFile(m_storePath+"/"+"raw_"+ current_time + ".txt");
        if(!p_datafile->open(QIODevice::WriteOnly)){
            QMessageBox::warning(this,"warning", "can't open file to save data",QMessageBox::Yes);
        }
        else{
            m_has_setup_newfile = true;
        }
        delete p_debugfile;
        p_debugfile = new QFile(m_storePath+"/" + "Debug_msg"+ current_time + ".txt");
        delete p_processed_file;
        p_processed_file = new QFile(m_storePath+"/" + "processed_"+ current_time + ".txt");
        p_processed_file->open(QIODevice::WriteOnly);
    }
    p_datafile->write(tmpdata);
    if(m_glazer_on){
        p_glazerfile->write(tmpdata);
    }

    //**********************************   数据包解析 ****************************************************
    //************* 数据包包含4个部分：包序号（4字节，每字节小端先发）+mark（4字节，mark位于最小端最小bit位上）+通道数据（32*4字节，每字节小端先发）+脱落数据（8字节，展开为ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n）
    //*************
    m_buffer.append(tmpdata);
    if(!m_find_head){
        //寻找包起始位置
        if(m_buffer.size()<m_packet_size*3){
            return;
        }
        int move_count=0;
        while (move_count<m_packet_size+1){
            QByteArray _nbyte =  m_buffer.left(m_packet_size+4);//取1个包加4字节数据
            unsigned int ch_number1 = static_cast<unsigned int>((_nbyte[0]&0xFF)|((_nbyte[1]&0xFF)<<8)|((_nbyte[2]&0xFF)<<16)|((_nbyte[3]&0xFF)<<24));
            unsigned int ch_number2 = static_cast<unsigned int>((_nbyte[m_packet_size]&0xFF)|((_nbyte[m_packet_size+1]&0xFF)<<8)|((_nbyte[m_packet_size+2]&0xFF)<<16)|((_nbyte[m_packet_size+3]&0xFF)<<24));
            if(ch_number1+1 == ch_number2){
                m_packet_number_last = ch_number1-1;
                break;
            }
            m_buffer.remove(0,1);
            move_count++;
        }
        if(move_count >= (m_packet_size+1)){
            qDebug()<<"Error: can not find data packet head";
            return;
        }
        m_find_head = true;
    }
    while(m_buffer.size()>=m_packet_size){//包长度：4+4+4*通道数+8=144。由于Tcp传输中自行拆包为不同长短的包进行传输，这里等待足够一包的数据量后进行处理
        //包序号检测
        QByteArray packet_number_4_byte =  m_buffer.left(4);
        QByteArray packet_msg_byte =  m_buffer.left(7).right(1);
        QByteArray packet_mark_byte =  m_buffer.left(8).right(1);
        unsigned int packet_number_now = static_cast<unsigned int>((packet_number_4_byte[0]&0xFF)|((packet_number_4_byte[1]&0xFF)<<8)|((packet_number_4_byte[2]&0xFF)<<16)|((packet_number_4_byte[3]&0xFF)<<24));
        if(m_packet_number_last+1!=packet_number_now){
            m_find_head = false;
            return;
        }
        //获取debug信息
        int debug_msg_length = static_cast<unsigned char>(packet_msg_byte[0]);
        if(debug_msg_length){
            if(m_buffer.size()<debug_msg_length){
                continue;//等待数据扩充
            }
            m_buffer.remove(0,8);
            QByteArray msg = m_buffer.left(debug_msg_length);
            p_debugfile->open(QIODevice::WriteOnly|QIODevice::Append);
            p_debugfile->write(msg);
            p_debugfile->close();
            m_buffer.remove(0,debug_msg_length);
            if(m_debug_on){
                debug_msg_queue.append(msg);
            }
            m_packet_number_last = packet_number_now;
            continue;
        }
        m_packet_number_last = packet_number_now;
        //获取一个字节，目前仅8bit中的最低位代表标记信息，1有效，其他位均为0，即不做处理
        bool ok;
        qint8 mark_tmp = static_cast<qint8>(packet_mark_byte.toHex().toInt(&ok,16));
        mark.append(mark_tmp);
        //统一移除前8个字节
        m_buffer.remove(0,8);
        //处理数据
        if(m_channel_number<32){
            for(int channel = 0 ; channel<m_channel_number; ++channel){
                 QByteArray _4byte =  m_buffer.left(4);//取4个字节
                 m_buffer.remove(0,4);//从buffer中删去读取的4个字节
                 int perchannel_data = (_4byte[0]&0x000000FF)|((_4byte[1]&0x000000FF)<<8)|((_4byte[2]&0x000000FF)<<16)|((_4byte[3]&0x000000FF)<<24);
                 double r4 = ((double)perchannel_data)/24.0;//24倍增益
                 data_from_wifi.append(r4);//将QBytearray转换成double存入队列
                 QByteArray tmp_data = QByteArray::number(r4,'f',2);
                 tmp_data.append(' ');
                 p_processed_file->write(tmp_data);
            }
            //读取8字节电极脱落数据,每字节8bit ,含8通道 p或n状态
            //elect_lead_off[64] 为：ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n
            QByteArray _8byte =  m_buffer.left(8);
            m_buffer.remove(0,8);
            for (int i=0; i<m_channel_number*2; ++i){
                elect_lead_off.append(_8byte[i/8] & (1<<i));
            }
        }
        else{
            int times_32channel = m_channel_number/32;
            for(int channel32count = 0; channel32count<times_32channel; ++channel32count){
                //填充通道数据
                for(int channel = 0 ; channel<32; ++channel){
                     QByteArray _4byte =  m_buffer.left(4);//取4个字节
                     m_buffer.remove(0,4);//从buffer中删去读取的4个字节
                     int perchannel_data = (_4byte[0]&0x000000FF)|((_4byte[1]&0x000000FF)<<8)|((_4byte[2]&0x000000FF)<<16)|((_4byte[3]&0x000000FF)<<24);
                     double r4 = ((double)perchannel_data)/24.0;//24倍增益
                     data_from_wifi.append(r4);//将QBytearray转换成double存入队列
                     QByteArray tmp_data = QByteArray::number(r4,'f',2);
                     tmp_data.append(' ');
                     p_processed_file->write(tmp_data);
                }
                //读取8字节电极脱落数据,每字节8bit ,含8通道 p或n状态
                //elect_lead_off[64] 为：ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n
                for (int i = 0; i<8; ++i){
                    bool ok;
                    qint8 _1byte = m_buffer.left(1).toHex().toInt(&ok,16);
                    m_buffer.remove(0,1);
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
        //移除校验
        m_buffer.remove(0,1);
    }
    //*************
    //***************************      数据包解析结束        ********************
    if(p_datafile->size()>100000000){
        p_datafile->close();
        m_has_setup_newfile = false;
    }

}

//从buffer中删除一定长度的数据，使得buffer第一个字节开始就是包序号。
void CommnicateBoard::find_head(){

}

void CommnicateBoard::set_store_path(QString path){
    m_storePath = path;
}

QString CommnicateBoard::set_glazer_on(bool on){
    QString filename;
    m_glazer_on = on;
    if(on){
        QString current_time = QDateTime::currentDateTime().toString("yyyy-MM-dd[hh-mm-ss]");
        filename = m_storePath+"/"+ current_time + "-glazer.txt";
        p_glazerfile = new QFile(filename);
        if(!p_glazerfile->open(QIODevice::WriteOnly)){
            QMessageBox::warning(this,"warning", "can't open file to save glazer data",QMessageBox::Yes);
        }
    }
    else{
        p_glazerfile->close();
        delete p_glazerfile;
    }
    return filename;
}

void CommnicateBoard::flush(){
     m_buffer.clear();
     mark.clear();
     data_from_wifi.clear();
     elect_lead_off.clear();
}
void CommnicateBoard::send_to_board(QByteArray data){
    if(m_connect_type == _wifi){
        p_tcp_new_connection->write(data);
        p_tcp_new_connection->waitForBytesWritten();
    }
    else if(m_connect_type == _serial_com){
        p_com->write(data);
    }
}

void CommnicateBoard::com_start(QSerialPort *_serial_port){
    m_connect_type = _serial_com;
    p_com = _serial_port;
    //此时就设置为板子启动
    m_has_start_board = true;
    emit signal_board_start();
    QMessageBox::information(this,"提示", "串口连接成功！",QMessageBox::Yes);
    //if(p_new_thread != nullptr){
    //    p_new_thread->exit(0);
    //    p_new_thread->wait();
    //    delete p_new_thread;
    //    p_new_thread = nullptr;
    //}
    //p_serial_com = new com_commnicate(port, this);
    //p_new_thread = new QThread();
    //connect(p_new_thread,SIGNAL(started()),this,SLOT(slot_com_prepare()));
    //p_serial_com->moveToThread(p_new_thread);
    //p_new_thread->start();
    connect(p_com, SIGNAL(readyRead()), this, SLOT(slot_read_data()));
}
void CommnicateBoard::com_stop(){
    //m_com_find_head = false;
    m_find_head = false;
    flush();
}
//void CommnicateBoard::slot_com_prepare(){
//    connect(p_com, SIGNAL(readyRead()), this, SLOT(slot_com_run()));
//}
/*
void CommnicateBoard::slot_com_run(){
    if(!tcp_obj->m_has_start_board){
        qDebug()<<"forbid running here";
    }
    else{
        //调试模式，接收发送命令的返回值
        if(tcp_obj->m_command_return_mode){
            //memset(com_buffer, 0, MAX_SIZE_COM_BUFFER*sizeof(char));
            //ReadFile(*com_handle,com_buffer,1,&wCount,NULL);
            QByteArray tmpdata = serial_port->readAll();
            int return_data = int(tmpdata[0]&0xFF);
            QString content = QString::number(return_data,16);
            tcp_obj->p_command_return_label->setText(content);
        }

        //接收逻辑
        //注：没有设置读取buffer大小，默认为无限制，适用于数据不能丢失场合。dataready信号在BUffer中有数据待读取和新接收到数据时均会触发。但会合并触发。
        //memset(com_buffer, 0, MAX_SIZE_COM_BUFFER*sizeof(char));
        //ReadFile(*com_handle,com_buffer,MAX_SIZE_COM_BUFFER,&wCount,NULL);
        QByteArray tmpdata = serial_port->readAll();

        //新建文件用于存储原始数据
        if(!tcp_obj->m_has_setup_newfile){
            delete tcp_obj->p_datafile;
            QString current_time = QDateTime::currentDateTime().toString("yyyy-MM-dd[hh-mm-ss]");
            tcp_obj->p_datafile = new QFile(tcp_obj->m_storePath+"/"+ current_time + ".txt");
            if(!tcp_obj->p_datafile->open(QIODevice::WriteOnly)){
                QMessageBox::warning(this,"warning", "can't open file to save data",QMessageBox::Yes);
            }
            else
                tcp_obj->m_has_setup_newfile = true;
        }
        tcp_obj->p_datafile->write(tmpdata);
        if(tcp_obj->m_glazer_on){
            tcp_obj->p_glazerfile->write(tmpdata);
        }
        //**********************************   数据包解析 ****************************************************
        //************* 数据包包含4个部分：包序号（4字节，每字节小端先发）+mark（4字节，mark位于最小端最小bit位上）+通道数据（32*4字节，每字节小端先发）+脱落数据（8字节，展开为ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n）
        //*************
        tcp_obj->m_buffer.append(tmpdata);
        if(!tcp_obj->m_com_find_head){
            //寻找包起始位置
            int _m_packet_size = tcp_obj->m_packet_size;
            if(tcp_obj->m_buffer.size()<_m_packet_size*3){
                return;
            }
            int move_count=0;
            while (move_count<_m_packet_size+1){
                QByteArray _nbyte =  tcp_obj->m_buffer.left(_m_packet_size+4);//取1个包加4字节数据
                unsigned int ch_number1 = (_nbyte[0]&0xFF)|((_nbyte[1]&0xFF)<<8)|((_nbyte[2]&0xFF)<<16)|((_nbyte[3]&0xFF)<<24);
                unsigned int ch_number2 = (_nbyte[_m_packet_size]&0xFF)|((_nbyte[_m_packet_size+1]&0xFF)<<8)|((_nbyte[_m_packet_size+2]&0xFF)<<16)|((_nbyte[_m_packet_size+3]&0xFF)<<24);
                if(ch_number1+1 == ch_number2){
                    m_packet_number_last = ch_number1;
                    tcp_obj->m_buffer.remove(0,_m_packet_size);
                    break;
                }
                tcp_obj->m_buffer.remove(0,1);
                move_count++;
            }
            if(move_count >= (_m_packet_size+1)){
                qDebug()<<"Error: can not find data packet head";
                return;
            }
            tcp_obj->m_com_find_head = true;
        }
        while(tcp_obj->m_buffer.size()>=tcp_obj->m_packet_size){//包长度：4+4+4*通道数+8=144。由于Tcp传输中自行拆包为不同长短的包进行传输，这里等待足够一包的数据量后进行处理
            //包序号检测
            QByteArray packet_number_4_byte =  tcp_obj->m_buffer.left(4);
            unsigned int packet_number_now = (packet_number_4_byte[0]&0xFF)|((packet_number_4_byte[1]&0xFF)<<8)|((packet_number_4_byte[2]&0xFF)<<16)|((packet_number_4_byte[3]&0xFF)<<24);;
            if(m_packet_number_last+1!=packet_number_now){
                tcp_obj->m_com_find_head = false;
                return;
            }
            m_packet_number_last = packet_number_now;
            tcp_obj->m_buffer.remove(0,4);
            //移除前3个字节冗余
            tcp_obj->m_buffer.remove(0,3);
            //获取一个字节，目前仅8bit中的最低位代表标记信息，1有效，其他位均为0，即不做处理
            bool ok;
            qint8 mark_tmp = tcp_obj->m_buffer.left(1).toHex().toInt(&ok,16);
            tcp_obj->m_buffer.remove(0,1);
            tcp_obj->mark.append(mark_tmp);
            if(tcp_obj->m_channel_number<32){
                for(int channel = 0 ; channel<tcp_obj->m_channel_number; ++channel){
                     QByteArray _4byte =  tcp_obj->m_buffer.left(4);//取4个字节
                     tcp_obj->m_buffer.remove(0,4);//从buffer中删去读取的4个字节
                     int perchannel_data = (_4byte[0]&0x000000FF)|((_4byte[1]&0x000000FF)<<8)|((_4byte[2]&0x000000FF)<<16)|((_4byte[3]&0x000000FF)<<24);
                     double r4 = ((double)perchannel_data)/24.0;//24倍增益
                     tcp_obj->data_from_wifi.append(r4);//将QBytearray转换成double存入队列
                }
                //读取8字节电极脱落数据,每字节8bit ,含8通道 p或n状态
                //elect_lead_off[64] 为：ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n
                QByteArray _8byte =  tcp_obj->m_buffer.left(8);
                tcp_obj->m_buffer.remove(0,8);
                for (int i=0; i<tcp_obj->m_channel_number*2; ++i){
                    tcp_obj->elect_lead_off.append(_8byte[i/8] & (1<<i));
                }
            }
            else{
                int times_32channel = tcp_obj->m_channel_number/32;
                for(int channel32count = 0; channel32count<times_32channel; ++channel32count){
                    //填充通道数据
                    for(int channel = 0 ; channel<32; ++channel){
                         QByteArray _4byte =  tcp_obj->m_buffer.left(4);//取4个字节
                         tcp_obj->m_buffer.remove(0,4);//从buffer中删去读取的4个字节
                         int perchannel_data = (_4byte[0]&0x000000FF)|((_4byte[1]&0x000000FF)<<8)|((_4byte[2]&0x000000FF)<<16)|((_4byte[3]&0x000000FF)<<24);
                         double r4 = ((double)perchannel_data)/24.0;//24倍增益
                         tcp_obj->data_from_wifi.append(r4);//将QBytearray转换成double存入队列
                    }
                    //读取8字节电极脱落数据,每字节8bit ,含8通道 p或n状态
                    //elect_lead_off[64] 为：ch1p,ch2p,ch3p,...,ch32p,ch1n,ch2n,...,ch32n
                    for (int i = 0; i<8; ++i){
                        bool ok;
                        qint8 _1byte = tcp_obj->m_buffer.left(1).toHex().toInt(&ok,16);
                        tcp_obj->m_buffer.remove(0,1);
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
            tcp_obj->m_buffer.remove(0,1);
        }
        //*************
        //***************************      数据包解析结束        ********************
        if(tcp_obj->p_datafile->size()>100000000){
            tcp_obj->p_datafile->close();
            tcp_obj->m_has_setup_newfile = false;
        }
    }
}
*/

void CommnicateBoard::set_debug_on(){
    m_debug_on = true;
    p_debug_thread = new QThread(this);
    p_debug_process = new DebugShow(debug_msg_queue);
    p_debug_process->moveToThread(p_debug_thread);
    //connect(p_debug_thread,SIGNAL(finished()),this,SLOT(slot_set_debug_off()));
    connect(p_debug_process,SIGNAL(signal_need_exit()),this,SLOT(slot_set_debug_off()));
    p_debug_thread->start();
    p_debug_process->run();
}

void CommnicateBoard::slot_set_debug_off(){
    //qDebug()<<"thread exit";
    m_debug_on = false;
    debug_msg_queue.clear();
    p_debug_thread->quit();
    p_debug_thread->wait();
}
