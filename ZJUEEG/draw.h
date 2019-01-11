#ifndef DRAW_H
#define DRAW_H


#include <QWidget>
#include <mainwindow.h>

class Draw : public QWidget
{
public:
    Draw(MainWindow* mainwindow, QWidget *parent = 0);
    void paint(QPaintEvent *);
private:
    MainWindow * window;
};


#endif // DRAW_H

