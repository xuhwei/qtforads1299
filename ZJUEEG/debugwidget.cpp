#include "debugwidget.h"
#include "ui_debugwidget.h"

DebugWidget::DebugWidget(QQueue<QByteArray>& _msg_queue, QWidget *parent) :
    QMainWindow(parent),    
    ui(new Ui::DebugWidget),
    msg_queue(_msg_queue)
{
    ui->setupUi(this);
    this->setWindowTitle("调试消息");
    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(slot_show_message()));
    timer->setInterval(1000);
    timer->start();
}

DebugWidget::~DebugWidget()
{
    delete ui;
    delete timer;
}
void DebugWidget::closeEvent(QCloseEvent*){
    emit signal_debug_widget_close();
}
void DebugWidget::slot_show_message(){
    //ui->textBrowser->append("tmp_msg");
    while(!msg_queue.isEmpty()){
        QByteArray tmp_msg = msg_queue.front();
        msg_queue.pop_front();
        ui->textBrowser->append(tmp_msg);
    }
}
void DebugWidget::slot_clear_widget(){
    ui->textBrowser->clear();
}
