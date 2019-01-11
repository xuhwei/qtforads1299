#include "draw.h"
#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>


Draw::Draw(MainWindow* mainwindow, QWidget *parent):QWidget(parent)
{
    //mainwindow->ui->frame;
    //window = mainwindow;
}
void Draw::paint(QPaintEvent *)
{
    QPixmap frame;
    QPainter painter(this);
    //int x = window->ui->frame->pos().x();
    //int y = window->ui->frame->pos().y();
    //painter.drawRect(x,y,100,20);
    //painter.setBackground();
    //painter.setRenderHint(QPainter::Antialiasing,true);
    //painter.drawPixmap();
    //QRect r(rect());
    //r.adjust(0, 0, -1, -1);
    //painter.drawRect(r);
}
/*
void Draw::paintEvent(QPaintEvent *)
{
   //void QPainter::drawPixmap(int x, int y, int width, int height, const QPixmap &pixmap)
   QPainter painter(this);
   int x1 = ui->view1Frame->pos().x();
   int y1 = ui->view1Frame->pos().y();
   painter.drawPixmap(x1,y1,ui->view1Frame->width(),ui->view1Frame->height(),image1);

   int x2 = ui->view2Frame->pos().x();
   int y2 = ui->view2Frame->pos().y();
   painter.drawPixmap(x2,y2,ui->view2Frame->width(),ui->view2Frame->height(),image2);

   int x3 = ui->view3Frame->pos().x();
   int y3 = ui->view3Frame->pos().y();
   painter.drawPixmap(x3,y3,ui->view3Frame->width(),ui->view3Frame->height(),image3);

   int x4 = ui->view4Frame->pos().x();
   int y4 = ui->view4Frame->pos().y();
   painter.drawPixmap(x4,y4,ui->view4Frame->width(),ui->view4Frame->height(),image4);
}
*/
