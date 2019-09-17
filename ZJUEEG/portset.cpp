#include "portset.h"
#include "ui_portset.h"

PortSet::PortSet(QVector<QString> &port_name, QSerialPort *port, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PortSet)
{
    ui->setupUi(this);
    serial_port = port;
    for(QVector<QString>::iterator it= port_name.begin(); it!= port_name.end(); ++it){
        ui->port_name->addItem(*it);
    }
    connect(this,SIGNAL(accepted()),this,SLOT(update_arg()));
}

PortSet::~PortSet()
{
    delete ui;
}

void PortSet::update_arg(){
    serial_port->close();
    serial_port->setPortName(ui->port_name->currentText());
    serial_port->setBaudRate(qint32(ui->bandrate->text().toInt()));
    switch (ui->data_bit->currentIndex()) {
    case 0:
        serial_port->setDataBits(QSerialPort::Data8);
        break;
    case 1:
        serial_port->setDataBits(QSerialPort::Data7);
        break;
    case 2:
        serial_port->setDataBits(QSerialPort::Data6);
        break;
    case 3:
        serial_port->setDataBits(QSerialPort::Data5);
        break;
    default:
        serial_port->setDataBits(QSerialPort::Data8);
        break;
    }
    switch (ui->stop_bit->currentIndex()) {
    case 0:
        serial_port->setStopBits(QSerialPort::TwoStop);
        break;
    case 1:
        serial_port->setStopBits(QSerialPort::OneStop);
        break;
    default:
        serial_port->setStopBits(QSerialPort::TwoStop);
        break;
    }
    serial_port->setParity(QSerialPort::NoParity);
    serial_port->setFlowControl(QSerialPort::NoFlowControl);
    emit portSetDone();
}
