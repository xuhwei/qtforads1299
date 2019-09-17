#ifndef PORTSET_H
#define PORTSET_H

#include <QDialog>
#include <QSerialPort>

namespace Ui {
class PortSet;
}

class PortSet : public QDialog
{
    Q_OBJECT

public:
    explicit PortSet(QVector<QString> &port_name, QSerialPort *port, QWidget *parent = 0);
    ~PortSet();
signals:
    void portSetDone();
private slots:
    void update_arg();
private:
    Ui::PortSet *ui;
    QSerialPort *serial_port;
};

#endif // PORTSET_H
