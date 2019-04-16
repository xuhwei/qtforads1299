#include "myframe.h"
#include <QMessageBox>

myframe::myframe(QWidget *parent):QWidget(parent)
{

    has_data_to_draw = false;
    x_interval = 0.0;
    y_scale =0.0;
    x_scale =0.0;
    pen_pos_x = 5;//画笔当前位置
    pen_pos_y = 5;
    polyline_first = QPointF(pen_pos_x,pen_pos_y);
    mark0 = false;
}

//myframe::~myframe(){
//    delete [] polyline;
//    delete [] polyline_2;
//}

void myframe::paintEvent(QPaintEvent*)
{
    QStylePainter stylePainter(this);
    stylePainter.drawPixmap(0, 0, pixmap);
    //qDebug("paintEvent");
}
void myframe::mousePressEvent(QMouseEvent*)
{
    int x = this->x();
    int y = this->y();
    int height = this->frameGeometry().height();
    int width = this->frameGeometry().width();
    qDebug("x=%d,y=%d,h=%d,w=%d",x,y,height,width);
    update();
}

int myframe::hasDataToDraw(double sample_rate, QVector<double>& singlechannel_data,
                           int amplititude_scale, int time_scale,
                           Qt::GlobalColor lineColor,
                           double min, double max , double rms,
                           QVector<qint8>& mark, bool arv,int arv_w_count,int arv_w_length,
                           bool leadoff_p, bool leadoff_n)
{
    //qDebug("hadDatoToDraw");
    QPainter painter(&pixmap);
    painter.initFrom(this);
    painter.setPen(lineColor);

    int length = singlechannel_data.size();//数据长度
    if (arv){
        length = arv_w_count;
    }
    if(length<2){
        qDebug("the number of data is less than 2");
        return -1;
    }   
    height = this->frameGeometry().height();//画布高
    width = this->frameGeometry().width();//画布宽

    polyline = new QPointF[length + 1];//需要和上一次绘图点连接，所以从上一次绘图最后一点开始绘图
    QVector<double> xposition;
    xposition.resize(length + 1);

    if(arv){//arv 将窗长内的所有点计算RMS得出一个结果，相当于采样率下降
        x_interval=1000.0/(sample_rate/arv_w_length);
    }
    else{
        x_interval=1000.0/sample_rate;//ms
    }
    y_scale= (double)amplititude_scale;
    x_scale= (double)time_scale ;
    has_data_to_draw = true;
    //计算每两个点跨越多少时间(ms)范围（px）
    double x_among_two_point = x_interval/(x_scale/(double)(width-10));
    //计算每像素点（px）代表多少幅度(mv)
    double y_perScale = 2*y_scale/(double)height;// 单位:uv/px
    //擦除矩形，每画一次polyline，将要画的PolyLine x轴范围内矩形擦除。
    QRect eraseBlock(rect());
    //eraseBlock.adjust(0,1,0,1);
    double start_point = pen_pos_x;
    double end_point = start_point + ((double)length)*x_among_two_point;
    double start_point_left, start_point_right, end_point_left, end_point_right;
    int remain_point = ((double)width-10.0-start_point)/x_among_two_point +1;//计算右边还能画下几个点,包括polyline_first
    if(remain_point<0){
        qDebug("remain_point out of range");
        clearParameter();
        return -2;
    }
    if(end_point>width-10)
    {
        if(end_point-start_point > width-10-10){
            //如果绘图长度超过了窗口长度，（留10px余量）那么提示换更大的时基
            QMessageBox warnning;
            warnning.setText("需要更大的时基");
            warnning.exec();
            return -3;
        }
        //分右边和左边两个擦除矩阵
        start_point_right = start_point;
        end_point_right = width;
        //左边擦除矩形
        start_point_left = 5.0;
        end_point_left = 5.0+ (length + 1 - remain_point)*x_among_two_point;       
        eraseBlock.setLeft(start_point_left);
        eraseBlock.setRight(end_point_left);
        painter.eraseRect(eraseBlock);
    }
    else{
        start_point_right = start_point;
        end_point_right = end_point;
    }
    eraseBlock.setLeft(start_point_right);
    eraseBlock.setRight(end_point_right);
    painter.eraseRect(eraseBlock);

    double next_x,next_y;
    if(end_point < width-10)
    {
        polyline[0] = polyline_first;
        xposition[0] = polyline_first.x();
        for(int i = 1; i<length + 1 ;i++)
        {
            next_x = pen_pos_x + x_among_two_point;
            xposition[i] = next_x;
            //qDebug("height = %d",height);
            if(arv)
                next_y = height -  singlechannel_data[i-1]/y_perScale;
            else
                next_y = (double)(height/2) -  singlechannel_data[i-1]/y_perScale;
            polyline[i] = QPointF(next_x,next_y);
            //qDebug("x=%f,y=%f",next_x,next_y);
            pen_pos_x = next_x;
            pen_pos_y = next_y;
        }
        painter.drawPolyline(polyline, length + 1);
        delete [] polyline;
        polyline_first = QPointF(pen_pos_x,pen_pos_y);
    }
    else
    {   //分右边和左边两部分画图
        //右边
        polyline[0] = polyline_first;
        xposition[0] = polyline_first.x();
        for(int i = 1; i<remain_point; i++)
        {
            next_x = pen_pos_x + x_among_two_point;
            xposition[i] = next_x;
            if(arv)
                next_y = height -  singlechannel_data[i-1]/y_perScale;
            else
                next_y = (double)(height/2) -  singlechannel_data[i-1]/y_perScale;
            polyline[i] = QPointF(next_x,next_y);
            pen_pos_x = next_x;
            pen_pos_y = next_y;
        }
        painter.drawPolyline(polyline, remain_point);


        //左边； 左边第一个点会重复绘制右边最后一个点.注：mark并不会再重复右边最后一个点
        polyline_2 = new QPointF[length + 2 - remain_point];
        polyline_first = QPointF(5,pen_pos_y);
        pen_pos_x = 5.0;
        polyline_2[0] = polyline_first;
        for(int i = 1; i<length + 2 - remain_point; i++)
        {
            next_x = pen_pos_x + x_among_two_point;
            xposition[remain_point-1+i] = next_x;
            if(arv)
                next_y = height -  singlechannel_data[remain_point-2+i]/y_perScale;
            else
                next_y = (double)(height/2) -  singlechannel_data[remain_point-2+i]/y_perScale;
            polyline_2[i] = QPointF(next_x,next_y);
            pen_pos_x = next_x;
            pen_pos_y = next_y;
        }
        painter.drawPolyline(polyline_2, length + 2 -remain_point);
        delete [] polyline;
        delete [] polyline_2;
        polyline_first = QPointF(pen_pos_x,pen_pos_y);
    }
    //Draw mark , number of point this time  ,plus last point in last drawing
    painter.setPen(Qt::red);
    if(mark0) {
        painter.drawEllipse(xposition[0],height-10,4,4);
        painter.drawEllipse(xposition[0],10,4,4);
        painter.drawLine(QPoint(xposition[0]+2,height-10),QPoint(xposition[0]+2,14));
    }
    for(int i = 1; i<length+1; ++i){
        if(mark[i-1]&0x01){
            painter.drawEllipse(xposition[i],height-10,4,4);
            painter.drawEllipse(xposition[i],10,4,4);
            painter.drawLine(QPoint(xposition[i]+2,height-10),QPoint(xposition[i]+2,14));
        }
    }
    mark0 = mark[length-1]&0x01;
    // Draw min max rms text
    painter.setPen(Qt::darkGreen);
    eraseBlock.setLeft(width-360);
    eraseBlock.setRight(width-10);
    eraseBlock.setTop(10);
    eraseBlock.setBottom(20);
    painter.eraseRect(eraseBlock);
    painter.drawText(width-360,20,"min:"+QString::number(min,10,2)+"uV");
    painter.drawText(width-240,20,"max:"+QString::number(max,10,2)+"uV");
    if(!arv){//arv启用时不显示rms
        painter.drawText(width-120,20,"rms:"+QString::number(rms,10,2)+"uV");
    }
    // Draw the 0V line,1/2 scale line,draw text
    painter.setPen(Qt::darkGray);
    painter.drawLine(QPoint(0,height/2),QPoint(width,height/2));
    painter.drawLine(QPoint(0,height/4),QPoint(width,height/4));
    painter.drawLine(QPoint(0,3*height/4),QPoint(width,3*height/4));
    if(arv){
        painter.drawText(10,height/2,QString::number(amplititude_scale)+"uV");
        painter.drawText(10,height/4,QString::number(amplititude_scale*3/2)+"uV");
        painter.drawText(10,3*height/4,QString::number(amplititude_scale/2)+"uV");
        painter.drawText(width-80,height-5,QString::number(amplititude_scale*2)+"uV");
    }
    else{
        painter.drawText(10,height/2,tr("0V"));
        painter.drawText(10,height/4,QString::number(amplititude_scale/2)+"uV");
        painter.drawText(10,3*height/4,"-"+QString::number(amplititude_scale/2)+"uV");
        painter.drawText(width-80,height-5,"+/-"+QString::number(amplititude_scale)+"uV");
    }

    //Draw electrode off message
    if(leadoff_p || leadoff_n){
        QRect elect_off_block(rect());
        elect_off_block.setTop(height/2-8);
        elect_off_block.setBottom(height/2+8);
        elect_off_block.setLeft(width-60);
        elect_off_block.setRight(width-10);
        painter.fillRect(elect_off_block,QBrush(Qt::red));
        painter.setPen(Qt::black);
        QString p_off = leadoff_p?"p":"";
        QString n_off = leadoff_n?"n":"";
        painter.drawText(width-60,height/2+6,p_off + n_off + tr("脱落！"));
    }

    // Draw border
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(3);
    QRect r(rect());
    r.adjust(0, 0, -1, -1);
    painter.setPen(pen);
    painter.drawRect(r);

    update();
    return 0;
}

void myframe::refreshPixmap()
{
    // 建立双缓冲pixmap
    pixmap =  QPixmap(size());
    pixmap.fill();

    QPainter painter(&pixmap);
    painter.initFrom(this);

    // 清除
    painter.eraseRect(rect());
    clearParameter();
    // 画边框
    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(2);
    painter.setPen(pen);
    QRect r(rect());
    r.adjust(0, 0, -1, -1);
    painter.drawRect(r);

    //qDebug("w:%d,h:%d",width,height);

    update();
}

void myframe::resizeEvent(QResizeEvent*) {
    refreshPixmap();
}
void myframe::clearParameter(){
    has_data_to_draw = false;
    x_interval = 0.0;
    y_scale =0.0;
    x_scale =0.0;
    pen_pos_x = 5;//画笔当前位置
    pen_pos_y = 5;
    polyline_first = QPointF(pen_pos_x,pen_pos_y);
}
