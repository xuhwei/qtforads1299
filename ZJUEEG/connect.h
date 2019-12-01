#ifndef COMMUNICATEBOARD_H
#define COMMUNICATEBOARD_H

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
#include <QThread>

#include "debugwidget.h"

//#define MAX_SIZE_COM_BUFFER 192

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
class DebugShow;

class CommnicateBoard:public QDialog
{    
    Q_OBJECT
public:
     explicit CommnicateBoard( QString ip_in, quint16 port_in,TcpType tcp_type, int N_channel, QLabel* label);
     ~CommnicateBoard();

     void wifi_client_connect_board();
     void wifi_server_connect_board();
     void set_store_path(QString );
     void wifi_update_port(quint16);
     void wifi_update_ip(QString);
     void set_channel_Arg(int ch_number);
     void flush();
     void send_to_board(QByteArray);
     void com_start(QSerialPort* _serial_port);
     void com_stop();
     void set_debug_on();
     QString set_glazer_on(bool);

     QTcpSocket *p_tcp_new_connection= nullptr;
     QTcpSocket *p_tcp_client= nullptr;
     QTcpServer *p_tcp_server= nullptr;
     QSerialPort *p_com= nullptr;
     QLabel* p_command_return_label= nullptr;
     QFile* p_datafile= nullptr;   //原始数据，存储一切
     QFile* p_glazerfile= nullptr;  //glazer数据
     QFile* p_debugfile=nullptr;    //调试数据
     QFile* p_processed_file= nullptr; //仅有用数据
     //DebugWidget *p_debug_widget=nullptr;
     QThread *p_debug_thread = nullptr;
     DebugShow *p_debug_process = nullptr;

     QQueue<qint8> mark;
     QQueue<double> data_from_wifi;
     QQueue<bool> elect_lead_off;
     QQueue<QByteArray> debug_msg_queue;

     int m_channel_number;
     bool m_has_start_board;
     bool m_command_return_mode;
     bool m_has_setup_newfile;
     bool m_glazer_on;
     bool m_debug_on;
     //bool m_com_find_head;
     bool m_find_head;
     int m_packet_size;
     unsigned int m_packet_number_last;
     QByteArray m_buffer;
     QString m_storePath;
     ConnectType m_connect_type;


signals:
     void signal_board_start();

private:    
     TcpType m_tcp_type;
     QString m_ip;
     quint16 m_port;
     //QThread *p_new_thread = nullptr;
     //com_commnicate *p_serial_com = nullptr;

     void find_head();

private slots:
     void slot_wifi_tcp_client_connect_success();
     void slot_wifi_tcp_client_connect_failed();
     void slot_wifi_tcp_client_find_host();
     void slot_wifi_tcp_client_read_data();
     void slot_wifi_tcp_server_new_connect();
     void slot_read_data();
     void slot_set_debug_off();
     //void slot_com_run();
     //void slot_com_prepare();
};
/*
class com_commnicate: public QDialog
{
    Q_OBJECT
public:
    com_commnicate(QSerialPort *port, CommnicateBoard *obj){
        serial_port = port;
        tcp_obj = obj;
        m_keep_run = true;
        //memset(com_buffer,0,MAX_SIZE_COM_BUFFER*sizeof(char));
        m_packet_number_last = 0;
    }
    //void slot_com_run();
    QSerialPort *serial_port= nullptr;
public slots:
    void slot_com_run();
    void slot_com_prepare();
private:    
    CommnicateBoard *tcp_obj= nullptr;

    unsigned int m_packet_number_last;
    bool m_keep_run;
    //char com_buffer[MAX_SIZE_COM_BUFFER];
    DWORD wCount;
};
*/

class DebugShow: public QObject
{
    Q_OBJECT
public:
    explicit DebugShow(QQueue<QByteArray>& msg_queue):
        debug_msg_queue(msg_queue){}
    ~DebugShow(){
        if(p_debug_widget) delete p_debug_widget;
    }
signals:
    void signal_need_exit();
public slots:
    void run(){
        p_debug_widget =new DebugWidget(debug_msg_queue);
        p_debug_widget->show();
        connect(p_debug_widget,SIGNAL(signal_debug_widget_close()),this,SLOT(slot_exit_widget()));
    }
    void slot_exit_widget(){
        emit signal_need_exit();
    }
private:
    DebugWidget *p_debug_widget=nullptr;
    QQueue<QByteArray>& debug_msg_queue;
};

#endif // CommnicateBoard_H
