#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <iostream>
#include <QTime>
#include <QColor>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("ZJU Bioelectricity Acquisition");
    this->setWindowIcon(QIcon(":/images/zju.png"));
    saveFilePath = "..";
    ip = "10.10.10.1";
    port = 61613;
    ui->Port_lineEdit->setText("61613");
    notchFilter = false;
    bandFilter = false;
    hilbert = false;
    arv = false;
    arv_w_count = 0;
    arv_w_length = 0;
    fl_bandPass = 1.0;
    fh_bandPass = 50.0;
    ampli_scale = 100;
    time_scale = 500;
    /*注：channel_number参数修改后需对应修改：
     * 1)ui中绘图部分窗口部件
     * 2)initVectorDrawFrame()
     * 3)initVectorQCheckBox()
     */
    channel_number = 32;
    per_channel_number = 100;
    sampleRate = 1000.0;
    running = false;
    runFileData = false;

    for(int i=0;i<channel_number;i++){
        channel_enable_map.append(true);
    }
    //给data,elec_off_p,elec_off_n分配空间
    data.resize(channel_number);
    for(int i=0;i<channel_number;i++){
        data[i].resize(per_channel_number);
    }
    elect_off_p.resize(channel_number);
    elect_off_n.resize(channel_number);
    mark.resize(per_channel_number);
    //初始化绘图部件容器
    initVectorDrawFrame();
    initVectorQCheckBox();

    dataProcess = new SignalProcess(channel_number,per_channel_number);
    tcp = new TcpConnect(ip,port,Tcp_server);

    ui->amplititude_comboBox->setCurrentIndex(3);
    connect(ui->openfile_action, &QAction::triggered, this, &MainWindow::openfile);
    readFile = new QFile();
}

MainWindow::~MainWindow()
{
    if(tcp->startBoard){
        QByteArray data_send;
        data_send.append(static_cast<char>(0xaa));
        data_send.append(static_cast<char>(0x06));
        data_send.append(static_cast<char>(0x00));
        tcp->newconnection->write(data_send);
    }
    delete readFile;
    delete tcp;
    delete dataProcess;
    delete ui;
}

//初始化绘图部件容器
void MainWindow::initVectorDrawFrame(){
    drawframe.resize(channel_number);
    drawframe[0] =  ui->widget_1;
    drawframe[1] =  ui->widget_2;
    drawframe[2] =  ui->widget_3;
    drawframe[3] =  ui->widget_4;
    drawframe[4] =  ui->widget_5;
    drawframe[5] =  ui->widget_6;
    drawframe[6] =  ui->widget_7;
    drawframe[7] =  ui->widget_8;
    drawframe[8] =  ui->widget_9;
    drawframe[9] =  ui->widget_10;
    drawframe[10] =  ui->widget_11;
    drawframe[11] =  ui->widget_12;
    drawframe[12] =  ui->widget_13;
    drawframe[13] =  ui->widget_14;
    drawframe[14] =  ui->widget_15;
    drawframe[15] =  ui->widget_16;
    drawframe[16] =  ui->widget_17;
    drawframe[17] =  ui->widget_18;
    drawframe[18] =  ui->widget_19;
    drawframe[19] =  ui->widget_20;
    drawframe[20] =  ui->widget_21;
    drawframe[21] =  ui->widget_22;
    drawframe[22] =  ui->widget_23;
    drawframe[23] =  ui->widget_24;
    drawframe[24] =  ui->widget_25;
    drawframe[25] =  ui->widget_26;
    drawframe[26] =  ui->widget_27;
    drawframe[27] =  ui->widget_28;
    drawframe[28] =  ui->widget_29;
    drawframe[29] =  ui->widget_30;
    drawframe[30] =  ui->widget_31;
    drawframe[31] =  ui->widget_32;
}
void MainWindow::initVectorQCheckBox(){
    checkbox.resize(channel_number);
    checkbox[0] = ui->checkBox;
    checkbox[1] = ui->checkBox_2;
    checkbox[2] = ui->checkBox_3;
    checkbox[3] = ui->checkBox_4;
    checkbox[4] = ui->checkBox_5;
    checkbox[5] = ui->checkBox_6;
    checkbox[6] = ui->checkBox_7;
    checkbox[7] = ui->checkBox_8;
    checkbox[8] = ui->checkBox_9;
    checkbox[9] = ui->checkBox_10;
    checkbox[10] = ui->checkBox_11;
    checkbox[11] = ui->checkBox_12;
    checkbox[12] = ui->checkBox_13;
    checkbox[13] = ui->checkBox_14;
    checkbox[14] = ui->checkBox_15;
    checkbox[15] = ui->checkBox_16;
    checkbox[16] = ui->checkBox_17;
    checkbox[17] = ui->checkBox_18;
    checkbox[18] = ui->checkBox_19;
    checkbox[19] = ui->checkBox_20;
    checkbox[20] = ui->checkBox_21;
    checkbox[21] = ui->checkBox_22;
    checkbox[22] = ui->checkBox_23;
    checkbox[23] = ui->checkBox_24;
    checkbox[24] = ui->checkBox_25;
    checkbox[25] = ui->checkBox_26;
    checkbox[26] = ui->checkBox_27;
    checkbox[27] = ui->checkBox_28;
    checkbox[28] = ui->checkBox_29;
    checkbox[29] = ui->checkBox_30;
    checkbox[30] = ui->checkBox_31;
    checkbox[31] = ui->checkBox_32;
}

/***************  UI 槽函数***********
 *   修改存储路径
 */
void MainWindow::changeSavePath()
{
    saveFilePath = QFileDialog::getExistingDirectory(this,tr("saveDir"),tr("c:/"),QFileDialog::ShowDirsOnly);
    ui->savepath_label->setText(saveFilePath);
    tcp->setStorePath(saveFilePath);
}
/***************  UI 槽函数***********
 *   使能陷波器
 */
void MainWindow::changeNotchFilter(){
    if(Qt::Checked == ui->notch_checkBox->checkState())
        notchFilter = true;
    else if(Qt::Unchecked == ui->notch_checkBox->checkState())
        notchFilter = false;
    QString a = notchFilter?"open":"close";
    ui->statusbar->showMessage("notchFilter: " + a);
    dataProcess->setNotchFilter(sampleRate,notchFilter);
}
/***************  UI 槽函数***********
 *   使能高通滤波器
 */
void MainWindow::changeHpassFilter(){
    if(Qt::Checked == ui->hpass_checkBox->checkState())
        dataProcess->setHighPassFilter(true);
    else if(Qt::Unchecked == ui->hpass_checkBox->checkState())
        dataProcess->setHighPassFilter(false);
}
/***************  UI 槽函数***********
 *   使能希尔伯特求取包络
 */
void MainWindow::changeHilbert(){
    if(Qt::Checked == ui->hilbert_checkBox->checkState())
        hilbert = true;
    else if(Qt::Unchecked == ui->notch_checkBox->checkState())
        hilbert = false;
    QString a = hilbert?"open":"close";
    ui->statusbar->showMessage("Hilbert: " + a);
    dataProcess->setHilbert(hilbert);
}
/***************  UI 槽函数***********
 *   使能带通滤波器
 */
void MainWindow::changeBandFilter(){
    if(Qt::Checked == ui->bandfilter_checkbox->checkState())
        bandFilter = true;
    else if(Qt::Unchecked == ui->notch_checkBox->checkState())
        bandFilter = false;
    QString a = bandFilter?"open":"close";
    ui->statusbar->showMessage("bandFilter: " + a);
    dataProcess->setBandFilter(sampleRate,bandFilter,fl_bandPass,fh_bandPass);
}

/***************  UI 槽函数***********
 *   改变带通滤波器上限截止频率
 */
void MainWindow::changeFhFilter(){
    switch (ui->comboBox_2->currentIndex()){
    case 0: fh_bandPass = 50.0-5.0;break;
    case 1: fh_bandPass = 80.0;break;
    case 2: fh_bandPass = 100.0;break;
    case 3: fh_bandPass = 200.0;break;
    case 4: fh_bandPass = 500.0;break;
    default:fh_bandPass = 10000.0;
    }
    ui->statusbar->showMessage("fh: " + QString::number(fh_bandPass));
    dataProcess->setBandFilter(sampleRate,bandFilter,fl_bandPass,fh_bandPass);
}
/***************  UI 槽函数***********
 *   改变带通滤波器下限截止频率
 */
void MainWindow::changeFlFilter(){
    switch (ui->comboBox->currentIndex()){
    case 0: fl_bandPass = 1.0+2.0;break;
    case 1: fl_bandPass = 20.0;break;
    case 2: fl_bandPass = 50.0;break;
    case 3: fl_bandPass = 100.0;break;
    case 4: fl_bandPass = 150.0;break;
    default:fl_bandPass = 0.0;
    }
    ui->statusbar->showMessage("fl: " + QString::number(fl_bandPass));
    dataProcess->setBandFilter(sampleRate,bandFilter,fl_bandPass,fh_bandPass);
}
/***************  UI 槽函数***********
 *   启用平均整流 ARV
 */
void MainWindow::changeArv(){
    if(Qt::Checked == ui->arv_checkBox->checkState())
        arv = true;
    else if(Qt::Unchecked == ui->arv_checkBox->checkState())
        arv = false;
    QString a = arv?"open":"close";
    ui->statusbar->showMessage("ARV: " + a);
    dataProcess->setArv(arv);
    arv_w_length = dataProcess->getArvWindowLength();
    int arv_w_length_cover = dataProcess->getArvWindowLengthOver();///待修改
    arv_w_count = per_channel_number/arv_w_length;
}
/***************  UI 槽函数***********
 *   获取TCP连接端口号
 */
void MainWindow::getPort(){
    port = static_cast<quint16>(ui->Port_lineEdit->text().toInt(nullptr,10));
    ui->statusbar->showMessage("Port= "+ QString::number(port,10));
}
/***************  UI 槽函数***********
 *   获取TCP连接IP
 */
void MainWindow::getIp(){
    ip = ui->IP_lineEdit->text();
    ui->statusbar->showMessage("IP="+ip);
}
/***************  UI 槽函数***********
 *   修改幅度量程
 */
void MainWindow::changeAmplititude(){
    switch(ui->amplititude_comboBox->currentIndex()){
    case 0: ampli_scale = 10;break;
    case 1: ampli_scale = 20;break;
    case 2: ampli_scale = 50;break;
    case 3: ampli_scale = 100;break;
    case 4: ampli_scale = 200;break;
    case 5: ampli_scale = 500;break;
    case 6: ampli_scale = 1000;break;
    case 7: ampli_scale = 2000;break;
    case 8: ampli_scale = 5000;break;
    case 9: ampli_scale = 10000;break;
    default: ampli_scale = 10;
    }
    ui->statusbar->showMessage("Amplititude Scale: " + QString::number(ampli_scale));
}
/***************  UI 槽函数***********
 *   修改时间量程
 */
void MainWindow::changeTimeScale(){
    switch(ui->time_comboBox->currentIndex()){
    case 0: time_scale = 500;break;
    case 1: time_scale = 1000;break;
    case 2: time_scale = 2000;break;
    case 3: time_scale = 5000;break;
    case 4: time_scale = 10000;break;
    case 5: time_scale = 20000;break;
    case 6: time_scale = 50000;break;
    default: time_scale = 50;
    }
    ui->statusbar->showMessage("Time Scale: " + QString::number(time_scale));
}
/***************  UI 槽函数***********
 *   修改绘图通道使能
 */
void MainWindow::changeChannelEnable(){
    QString enableChannel = "Enable channel:";
    for(int channel=0; channel<channel_number; channel++){
        if(Qt::Checked == checkbox[channel]->checkState()){
            channel_enable_map[channel] = true;
            drawframe[channel]->setVisible(true);
        }
        else{
            channel_enable_map[channel] = false;
            drawframe[channel]->refreshPixmap();
            drawframe[channel]->setVisible(false);
        }
    }

    for(int i=0; i<channel_number; i++){
        if(channel_enable_map[i]){
            enableChannel += (QString::number(i+1,10)+" ");
        }
    }
    ui->statusbar->showMessage(enableChannel);
}
/***************  UI 槽函数***********
 *   启动TCP连接
 */
void MainWindow::connectWifi(){
    delete tcp;
    tcp = new TcpConnect(ip,port,Tcp_server);
    tcp->server_connectToBoard();
    tcp->setStorePath(saveFilePath);
    /*如果是客户端
     * tcp = new TcpConnect(ip,port,Tcp_client);
     * tcp->client_connectToBoard();
     */

}
/***************  UI 槽函数***********
 *   停止绘图并停止板子读取数据
 */
void MainWindow::stop_m(){
    running = false;
    if(tcp->startBoard){
        QByteArray data_send;
        data_send.append(static_cast<char>(0xaa));
        data_send.append(static_cast<char>(0x06));
        data_send.append(static_cast<char>(0x00));
        tcp->newconnection->write(data_send);
    }
}
/***************  UI 槽函数***********
 *   开始绘图
 */
void MainWindow::run_m(){
    running = true;

    if(!tcp->startBoard){
        /***************   使用文件读取的数据绘图     *******************/
        if(runFileData){
            QMessageBox::information(this,"","将使用选中文件内数据绘图");
            unsigned int numberToRead = static_cast<unsigned int>((4 + 4 + channel_number*4 +8)*per_channel_number);
            while(running){
                //读取依次为 4字节编号，4字节冗余（最后一个bit为mark），32*4字节通道数据，8字节电极脱落数据               
                char _32_channel_data[numberToRead];
                if(numberToRead != readFile->read(_32_channel_data,numberToRead)){
                    QMessageBox::information(this,"","读取结束");
                    readFile->close();
                    running = false;
                    runFileData = false;
                }
                else{
                    //初始化脱落检测容器
                    for(int channel = 0; channel<channel_number; ++channel){
                        elect_off_p[channel] = false;
                        elect_off_n[channel] = false;
                    }
                    //读取数据
                    QQueue<bool> elect_off_bool;
                    for(int j = 0; j< per_channel_number ;++j){
                        //处理电极脱落部分，8字节共计64bit 转为64个布尔值
                        for(int _8byte = 0; _8byte<8; ++_8byte){
                            unsigned char byte = static_cast<unsigned char>(_32_channel_data[(4 + 4 + channel_number*4 +8)*j + 4 + 4 + channel_number*4 + _8byte]);
                            elect_off_bool.append(byte&0x01);
                            elect_off_bool.append(byte&0x02);
                            elect_off_bool.append(byte&0x04);
                            elect_off_bool.append(byte&0x08);
                            elect_off_bool.append(byte&0x10);
                            elect_off_bool.append(byte&0x20);
                            elect_off_bool.append(byte&0x40);
                            elect_off_bool.append(byte&0x80);
                        }
                        for(int channel = 0; channel<channel_number; ++channel){
                            //补码转换为有符号int，直接转换
                            int perchannel_data = (_32_channel_data[j*(channel_number*4+16)+8+channel*4]&0x000000FF)|
                                                 ((_32_channel_data[j*(channel_number*4+16)+8+channel*4+1]&0x000000FF)<<8)|
                                                 ((_32_channel_data[j*(channel_number*4+16)+8+channel*4+2]&0x000000FF)<<16)|
                                                 ((_32_channel_data[j*(channel_number*4+16)+8+channel*4+3]&0x000000FF)<<24);
                            data[channel][j] = static_cast<double>(perchannel_data)/24.0;//增益24
                            elect_off_p[channel] =  elect_off_p[channel]|| elect_off_bool.front();
                            elect_off_bool.pop_front();
                            elect_off_n[channel_number - channel - 1] =  elect_off_n[channel_number - channel - 1]|| elect_off_bool.back();
                            elect_off_bool.pop_back();
                        }
                        mark[j] = _32_channel_data[(4 + 4 + channel_number*4 +8)*j + 7];
                    }

                    dataProcess->runProcess(data);
                    for(int channel = 0; channel < channel_number; ++channel){
                        if(channel_enable_map[channel]){
                            if(drawframe[channel]->hasDataToDraw(sampleRate, dataProcess->y_out[channel],
                                                                 ampli_scale, time_scale,
                                                                 channel%2 ? Qt::red : Qt::blue,
                                                                 dataProcess->min[channel],dataProcess->max[channel],dataProcess->rms[channel],
                                                                 mark,arv,arv_w_count,arv_w_length,
                                                                 elect_off_p[channel],elect_off_n[channel])
                                    <0){
                                stop_m();
                                break;
                            }
                        }
                    }
                    qApp->processEvents();
                    QTime timer = QTime::currentTime().addMSecs(100);
                    while( QTime::currentTime() < timer )
                        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                }
            }
        }
        /**************   生成模拟正弦波并画图*******************/
        else{
            QMessageBox::information(this,"","将使用模拟信号绘图");
            //产生10Hz递增,带100uV直流,幅度100uV正弦信号，采样率为1kHz
            for(int i = 0; i<channel_number; ++i){
                for(int j = 0; j< per_channel_number ;j++){
                    double y = 100.0*sin(2*PI*0.01*(i+1)*static_cast<double>(j))+100.0;
                    data[i][j] = y;
                }
            }
            for(int i = 0; i<per_channel_number; ++i){
                mark[i] = 0;
            }
            mark[static_cast<int>(per_channel_number/2)] = 1;

            while(running){
                dataProcess->runProcess(data);
                for(int channel = 0; channel < channel_number; ++channel){
                    if(channel_enable_map[channel]){
                        if(drawframe[channel]->hasDataToDraw(sampleRate, dataProcess->y_out[channel],
                                                             ampli_scale, time_scale,
                                                             channel%2 ? Qt::red : Qt::blue,
                                                             dataProcess->min[channel],dataProcess->max[channel],dataProcess->rms[channel],
                                                             mark,arv,arv_w_count,arv_w_length,
                                                             channel%2 ? true:false, false)<0)
                        {
                            stop_m();
                            break;
                        }
                    }
                }
                qApp->processEvents();
                //int delay = (int)((49.0/500000.0)*(double)time_scale);
                QTime timer = QTime::currentTime().addMSecs(25);
                while( QTime::currentTime() < timer )
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }
        }
    }
    else{
        /**************   启动接收 并绘图******************
         */

        QByteArray data_send;
        data_send.append(static_cast<char>(0xaa));
        data_send.append(static_cast<char>(0x06));
        data_send.append(static_cast<char>(0x01));
        tcp->newconnection->write(data_send);

        while(running){
             if(tcp->data_from_wifi.size() > per_channel_number*channel_number){
                 for(int channel = 0; channel<channel_number; ++channel){
                     elect_off_p[channel] = false;
                     elect_off_n[channel] = false;
                 }
                 for(int i=0; i<per_channel_number;i++){
                     for(int channel = 0; channel<channel_number; ++channel){
                         data[channel][i] = static_cast<double>(tcp->data_from_wifi.front());
                         tcp->data_from_wifi.pop_front();

                         elect_off_p[channel] = elect_off_p[channel] || tcp->elect_lead_off.front();
                         tcp->elect_lead_off.pop_front();
                     }
                     for(int channel = 0; channel<channel_number; ++channel){
                         elect_off_n[channel] = elect_off_n[channel] || tcp->elect_lead_off.front();
                         tcp->elect_lead_off.pop_front();
                     }
                     mark[i] = tcp->mark.front();
                     tcp->mark.pop_front();
                 }

                 dataProcess->runProcess(data);
                 for(int channel = 0; channel<channel_number; ++channel){
                     if(channel_enable_map[channel]){
                         if(drawframe[channel]->hasDataToDraw(sampleRate, dataProcess->y_out[channel],
                                                              ampli_scale, time_scale,
                                                              channel%2 ? Qt::red : Qt::blue ,
                                                              dataProcess->min[channel],dataProcess->max[channel],dataProcess->rms[channel],
                                                              mark,arv,arv_w_count,arv_w_length,
                                                              elect_off_p[channel], elect_off_n[channel])
                                 <0){
                             stop_m();
                             break;
                         }
                     }
                 }
              }
              qApp->processEvents();
        }
    }
}

/***************  UI 槽函数***********
 *   向板子发送命令
 */
void MainWindow::sendCommand(){
    if(tcp->startBoard){
        QByteArray c_send;
        QStringList command = ui->command_lineEdit->text().split(" ");
        if(command.size()>1){
            for(int i = 0; i<command.size(); ++i){
                bool ok;
                c_send.append(static_cast<char>(command[i].toInt(&ok,16)));
            }
            tcp->newconnection->write(c_send);
        }
    }
    else{
        QMessageBox tip;
        tip.setText("请先连接目标板");
        tip.exec();
    }
}
/***************  重写X按钮事件***********
 *   实现关闭程序时，如果还连接着板子，则给板子发送停止采集数据指令
 *   解决关闭程序时，如果绘制还未完成，后台程序未退出问题
 */
void MainWindow::closeEvent(QCloseEvent * event){
    if(tcp->startBoard){
        QByteArray data_send;
        data_send.append(static_cast<char>(0xaa));
        data_send.append(static_cast<char>(0x06));
        data_send.append(static_cast<char>(0x00));
        tcp->newconnection->write(data_send);
    }
    running = false;
}

/*************   打开文件  **************
 *
 */
void MainWindow::openfile(){
    QString filePath = QFileDialog::getOpenFileName();
    if(QMessageBox::Ok == QMessageBox::information(this,"确认选择的文件",QString(tr("选中的文件为："))+filePath,QMessageBox::Ok|QMessageBox::Cancel,QMessageBox::Cancel)){
        if(tcp->startBoard){
            QMessageBox::warning(this,"警告","目前正通过无线接收数据，请先断开连接再尝试操作");
        }
        else{
            runFileData = true;
            readFilePath = filePath;
            delete readFile;
            readFile = new QFile(filePath);
            if(!readFile->open(QIODevice::ReadOnly)){
                QMessageBox::warning(this,"警告","打开文件失败");
                runFileData = false;
            }
        }
    }
    else{
        QMessageBox::information(this,"","请重新选择文件",QMessageBox::Ok);
        runFileData = false;
    }
}



