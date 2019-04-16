#ifndef MYFRAME_H
#define MYFRAME_H

#include <QMainWindow>
#include <QPixmap>
#include <QPaintEvent>
#include <QPainter>
#include <qdebug.h>
#include <QStylePainter>
#include <QResizeEvent>

class myframe: public QWidget
{
public:
    explicit myframe(QWidget *parent = 0);
    //~myframe();

    int hasDataToDraw(double sample_rate, QVector<double>& singlechannel_data,
                      int amplititude_scale, int time_scale,
                      Qt::GlobalColor lineColor,
                      double min, double max, double rms,
                      QVector<qint8>& mark, bool arv, int arv_w_count,int arv_w_length,
                      bool leadoff_p = false, bool leadoff_n = false);
    void refreshPixmap();
protected:
    /*      重载
     *    重载resizeEvent，窗口变化时执行操作
     *    重载paintEvent，自定义器件绘图事件重载，所以绘图须在该函数中实现，采用双缓冲，将绘图内容存入PIXMAP，再在该事件函数中绘制PIXMAP
     *    重载mousePressEvent，保留待用。
     */
    void resizeEvent(QResizeEvent*);
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent*);
private:
    void clearParameter();//窗口大小变化时初始化绘图参数

    QPixmap pixmap;
    QPointF* polyline;
    QPointF* polyline_2;
    QPointF polyline_first;
    bool mark0;//存储上一次绘图终点mark值
    bool has_data_to_draw;
    double x_interval;//ms  每两个点的间距
    double y_scale;//uV
    double x_scale;//ms
    double pen_pos_x;//画笔当前位置
    double pen_pos_y;
    int height;
    int width;

};

#endif // MYFRAME_H
