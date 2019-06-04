/* 程序运行先设定通道数
 * 通道数默认为32通道，支持最高通道数为256
 *
 *
 */


#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <iostream>
#include <QColor>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("ZJU Bioelectricity Acquisition");
    this->setWindowIcon(QIcon(":/images/zju.png"));

    //先确定通道数
    QDialog *ch_N_Dialog = new QDialog;
    QVBoxLayout *vBoxLayout1 = new QVBoxLayout;
    QLabel *label1 = new QLabel;
    label1->setText("选择通道数，选择后无法更改");
    QComboBox *comboBox1 = new QComboBox;
    comboBox1->addItem("64");comboBox1->addItem("32");comboBox1->addItem("96");
    comboBox1->addItem("128");comboBox1->addItem("160");comboBox1->addItem("192");
    comboBox1->addItem("224");comboBox1->addItem("256");
    vBoxLayout1->addWidget(label1);
    vBoxLayout1->addWidget(comboBox1);
    ch_N_Dialog->setLayout(vBoxLayout1);
    connect(comboBox1,SIGNAL(currentIndexChanged(int)),this,SLOT(changeChannelNumber(int)));
    comboBox1->setCurrentIndex(1);
    ch_N_Dialog->show();ch_N_Dialog->exec();
    delete comboBox1;
    delete label1;
    delete vBoxLayout1;
    delete ch_N_Dialog;

    ip = "10.10.10.1";
    port = 61613;
    ui->Port_lineEdit->setText("61613");
    notchFilter = false;
    bandFilter = false;
    hpassFilter = false;
    rms_enable = false;
    rms_w_count = 0;
    rms_w_length = 0;
    fl_bandPass = 1.0;
    fh_bandPass = 50.0;
    per_channel_number = 100;//为了计算FFT,请保证能整除1000；不推荐修改
    sampleRate = 1000.0;
    running = false;
    runFileData = false;
    singleElectMode = false;

    channel_enable_map.resize(channel_number);
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
    tcp = new TcpConnect(ip,port,Tcp_server,channel_number,ui->command_return_label);
    connect(this->tcp,SIGNAL(boardStart()),this,SLOT(connectToBardDone()));

    ui->amplititude_comboBox->setCurrentIndex(3);
    ui->time_comboBox->setCurrentIndex(2);
    ui->sampleRate_comboBox->setCurrentIndex(2);
    ui->stopButton->setEnabled(false);
    ui->hpass_checkBox->setCheckState(Qt::CheckState::Checked);
    hpassFilter = true;
    ui->glazer_start->setEnabled(false);
    ui->glazer_stop->setEnabled(false);
    ui->glazer_result->setEnabled(false);
    ui->sampleRate_comboBox->setEnabled(false);
    ui->electmode_pushButton->setEnabled(false);
    ui->command_lineEdit->setEnabled(false);
    ui->command_send_botton->setEnabled(false);
    connect(ui->sampleRate_comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(changeSampleRate()));
    connect(ui->openfile_action, &QAction::triggered, this, &MainWindow::openfile);
    connect(ui->setChannel,SIGNAL(triggered(bool)),this,SLOT(openChannelSetWidget()));
    connect(ui->setFFT,SIGNAL(triggered(bool)),this,SLOT(openFFTSetWidget()));
    connect(ui->stft,SIGNAL(triggered(bool)),this,SLOT(openSTFTWidget()));
    readFile = new QFile();

    fft_selfControl = true;
    fft_xmin = fft_xmax = fft_ymin = fft_ymax = 0;
    initFFTWidget(true,fft_xmin,fft_xmax,fft_ymin,fft_ymax);
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
    for(int i=1; i<=channel_number;++i){
        QString obj_name_frame = "widget_"+QString::number(i);
        myframe *p_frame = ui->tabWidget->findChild<myframe *>(obj_name_frame);
        drawframe[i-1] = p_frame;
    }
}
void MainWindow::initVectorQCheckBox(){
    checkbox.resize(channel_number);
    for(int i=1; i<=channel_number;++i){
        QString obj_name_checkbox = "checkBox_"+QString::number(i);
        QCheckBox *p_checkbox = ui->tabWidget->findChild<QCheckBox *>(obj_name_checkbox);
        checkbox[i-1] = p_checkbox;
        connect(checkbox[i-1],SIGNAL(stateChanged(int)),this,SLOT(changeChannelEnable()));
    }
    //默认使能前8个通道
    for(int i=8; i<channel_number; ++i){
        checkbox[i]->setChecked(false);
    }
}

/***************  UI 槽函数***********
 *   修改存储路径
 */
void MainWindow::changeSavePath()
{
    QString saveFilePath = QFileDialog::getExistingDirectory(this,tr("saveDir"),tr("c:/"),QFileDialog::ShowDirsOnly);
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
        hpassFilter = true;
    else if(Qt::Unchecked == ui->hpass_checkBox->checkState())
        hpassFilter = false;
    dataProcess->setHighPassFilter(sampleRate,hpassFilter);
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
    case 0: fh_bandPass = 50.0;break;
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
    case 0: fl_bandPass = 1.0;break;
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
 *   启用RMS
 */
void MainWindow::changeRMS(){
    if(Qt::Checked == ui->rms_checkBox->checkState()){
        rms_enable = true;
        ui->glazer_start->setEnabled(true);
    }
    else if(Qt::Unchecked == ui->rms_checkBox->checkState()){
        rms_enable = false;
        ui->glazer_start->setEnabled(false);
        ui->glazer_stop->setEnabled(false);
        ui->glazer_result->setEnabled(false);
    }
    for(int channel = 0; channel <channel_number;channel++){
        drawframe[channel]->setRMS(rms_enable);
        drawframe[channel]->refreshPixmap();
    }
    QString a = rms_enable?"open":"close";
    ui->statusbar->showMessage("RMS: " + a);
    dataProcess->setRMS(rms_enable);
    rms_w_length = dataProcess->getRMSWindowLength();
    //int rms_w_length_cover = dataProcess->getRMSWindowLengthOver();///待修改
    rms_w_count = per_channel_number/rms_w_length;
}
/***************  UI 槽函数***********
 *   glazer评估启动标识
 */
void MainWindow::glazerStart(){
    glazerfilename = tcp->setGlazerOnOff(true);
    ui->glazer_start->setEnabled(false);
    ui->glazer_stop->setEnabled(true);
    ui->glazer_result->setEnabled(false);
    glazertimer = new TimeCount(ui->timer_label);
    glazertimer->start();
}
/***************  UI 槽函数***********
 *   glazer评估结束标识
 */
void MainWindow::glazerEnd(){
    tcp->setGlazerOnOff(false);
    ui->glazer_start->setEnabled(true);
    ui->glazer_stop->setEnabled(false);
    ui->glazer_result->setEnabled(true);
    glazertimer->stopRun();
    glazertimer->quit();
    glazertimer->wait();
    delete glazertimer;
    stop_m();
}
/***************  UI 槽函数***********
 *   glazer评估输出结果
 */
void MainWindow::glazerResult(){
    glazer_window = new Glazer(glazerfilename,sampleRate);
    glazer_window->show();
}
/***************  UI 槽函数***********
 *   获取TCP连接端口号
 */
void MainWindow::getPort(){
    port = static_cast<quint16>(ui->Port_lineEdit->text().toInt(nullptr,10));
    ui->statusbar->showMessage("Port= "+ QString::number(port,10));
    tcp->updatePort(port);
}
/***************  UI 槽函数***********
 *   获取TCP连接IP
 */
void MainWindow::getIp(){
    ip = ui->IP_lineEdit->text();
    ui->statusbar->showMessage("IP="+ip);
    tcp->updateIp(ip);
}
/***************  UI 槽函数***********
 *   修改幅度量程
 */
void MainWindow::changeAmplititude(){
    int ampli_scale;
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
    default: ampli_scale = 100;
    }
    for(int channel = 0; channel <channel_number;channel++){
        drawframe[channel]->setYscale(ampli_scale);
        drawframe[channel]->refreshScale();
    }
    ui->statusbar->showMessage("Amplititude Scale: " + QString::number(ampli_scale));
}
/***************  UI 槽函数***********
 *   修改时间量程
 */
void MainWindow::changeTimeScale(){
    int time_scale;
    switch(ui->time_comboBox->currentIndex()){
    case 0: time_scale = 500;break;
    case 1: time_scale = 1000;break;
    case 2: time_scale = 2000;break;
    case 3: time_scale = 5000;break;
    case 4: time_scale = 10000;break;
    case 5: time_scale = 20000;break;
    case 6: time_scale = 50000;break;
    default: time_scale = 1000;
    }
    for(int channel = 0; channel <channel_number;channel++){
        drawframe[channel]->setXscale(time_scale);
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
 *   连接板子后触发该函数
 */
void MainWindow::connectToBardDone(){
    ui->IP_lineEdit->setEnabled(false);
    ui->Port_lineEdit->setEnabled(false);
    ui->sampleRate_comboBox->setEnabled(true);
    ui->electmode_pushButton->setEnabled(true);
    ui->command_lineEdit->setEnabled(true);
    ui->command_send_botton->setEnabled(true);
}
/***************  UI 槽函数***********
 *   更改采样率
 */
void MainWindow::changeSampleRate(){
    int index = 0x90;
    switch(ui->sampleRate_comboBox->currentIndex()){
    case 0: sampleRate = 250.0; index+=6;break;
    case 1: sampleRate = 500.0; index+=5;break;
    case 2: sampleRate = 1000.0;index+=4;break;
    case 3: sampleRate = 2000.0;index+=3;break;
    case 4: sampleRate = 4000.0;index+=2;break;
    case 5: sampleRate = 8000.0;index+=1;break;
    case 6: sampleRate = 16000.0;break;
    default: sampleRate = 1000.0;index+=4;
    }
    ui->statusbar->showMessage("SampleRate: " + QString::number(sampleRate,'f',1)+"Hz");
    QByteArray data_send;
    data_send.append(static_cast<char>(0xaa));
    data_send.append(static_cast<char>(0x03));
    data_send.append(static_cast<char>(0x01));
    data_send.append(static_cast<char>(index));
    tcp->newconnection->write(data_send);

    dataProcess->setBandFilter(sampleRate,bandFilter,fl_bandPass,fh_bandPass);
    dataProcess->setNotchFilter(sampleRate,notchFilter);
    dataProcess->setHighPassFilter(sampleRate,hpassFilter);
}
/***************  UI 槽函数***********
 *   启动TCP连接
 */
void MainWindow::connectWifi(){
    tcp->server_connectToBoard();
    /*如果是客户端
     * tcp = new TcpConnect(ip,port,Tcp_client);
     * tcp->client_connectToBoard();
     */
}
/***************  UI 槽函数***********
 *   切换单 、双电极接法
 */
void MainWindow::changeSingleBipolarElect(){
    if(tcp->startBoard){
        if(!singleElectMode){
            int channel_addr = 0x05;
            for(int i = 0 ;i <8; ++i){
                QByteArray data_send;
                data_send.append(static_cast<char>(0xaa));
                data_send.append(static_cast<char>(0x03));
                data_send.append(static_cast<char>(channel_addr));
                data_send.append(static_cast<char>(0x68));
                tcp->newconnection->write(data_send);
                channel_addr++;
                QTime timer = QTime::currentTime().addMSecs(25);
                while( QTime::currentTime() < timer )
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }
            singleElectMode = true;
            ui->electmode_pushButton->setText("切换为双电极模式（目前为单电极）");
        }
        else{
            int channel_addr = 0x05;
            for(int i = 0 ;i <8; ++i){
                QByteArray data_send;
                data_send.append(static_cast<char>(0xaa));
                data_send.append(static_cast<char>(0x03));
                data_send.append(static_cast<char>(channel_addr));
                data_send.append(static_cast<char>(0x60));
                tcp->newconnection->write(data_send);
                channel_addr++;
                QTime timer = QTime::currentTime().addMSecs(25);
                while( QTime::currentTime() < timer )
                    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            }
            singleElectMode = false;
            ui->electmode_pushButton->setText("切换为单电极模式（目前为双电极）");
        }
    }
    else{
        QMessageBox::information(this,"","请先连接目标板");
    }
}
/***************  UI 槽函数***********
 *   停止绘图并停止板子读取数据
 */
void MainWindow::stop_m(){
    running = false;
    ui->runButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->command_return_checkBox->setEnabled(true);
    if(rms_enable && ui->glazer_stop->isEnabled()){
        glazerEnd();
    }
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
    ui->runButton->setEnabled(false);
    ui->stopButton->setEnabled(true);
    ui->command_return_checkBox->setCheckable(false);
    ui->command_return_checkBox->setEnabled(false);
    tcp->commandReturn = false;

    if(!tcp->startBoard){
        /***************   使用文件读取的数据绘图     *******************/
        if(runFileData){
            QMessageBox::information(this,"","将使用选中文件内数据绘图");
            unsigned int numberToRead = static_cast<unsigned int>((4 + 4 + (32*4 +8)*(channel_number/32))*per_channel_number);
            while(running){
                //读取依次为 4字节编号，4字节冗余（最后一个bit为mark），channel_number*4字节通道数据，8字节电极脱落数据
                char _32_channel_data[numberToRead];
                int boradNumber = channel_number/32;
                if(numberToRead != readFile->read(_32_channel_data,numberToRead)){
                    QMessageBox::information(this,"","读取结束");
                    readFile->close();
                    running = false;
                    runFileData = false;
                    ui->runButton->setEnabled(true);
                    ui->stopButton->setEnabled(false);
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
                        for(int _32count = 0; _32count < boradNumber; ++_32count){
                            //处理电极脱落部分，8字节共计64bit 转为64个布尔值
                            for(int _8byte = 0; _8byte<8; ++_8byte){
                                unsigned char byte = static_cast<unsigned char>(_32_channel_data[(8 + (32*4 +8)*boradNumber)*j+ 8 + (32*4 +8)*_32count + _8byte]);
                                elect_off_bool.append(byte&0x01);
                                elect_off_bool.append(byte&0x02);
                                elect_off_bool.append(byte&0x04);
                                elect_off_bool.append(byte&0x08);
                                elect_off_bool.append(byte&0x10);
                                elect_off_bool.append(byte&0x20);
                                elect_off_bool.append(byte&0x40);
                                elect_off_bool.append(byte&0x80);
                            }
                            for(int channel = 0; channel<32; ++channel){
                                //补码转换为有符号int，直接转换
                                int perchannel_data = (_32_channel_data[j*(8 + (32*4 +8)*boradNumber)+8+channel*4]&0x000000FF)|
                                                     ((_32_channel_data[j*(8 + (32*4 +8)*boradNumber)+8+channel*4+1]&0x000000FF)<<8)|
                                                     ((_32_channel_data[j*(8 + (32*4 +8)*boradNumber)+8+channel*4+2]&0x000000FF)<<16)|
                                                     ((_32_channel_data[j*(8 + (32*4 +8)*boradNumber)+8+channel*4+3]&0x000000FF)<<24);
                                data[32*_32count+channel][j] = static_cast<double>(perchannel_data)/24.0;//增益24
                                elect_off_p[32*_32count+channel] =  elect_off_p[32*_32count+channel]|| elect_off_bool.front();
                                elect_off_bool.pop_front();
                                elect_off_n[channel_number - 32*_32count - channel - 1] =  elect_off_n[channel_number - 32*_32count - channel - 1]|| elect_off_bool.back();
                                elect_off_bool.pop_back();
                            }
                            mark[j] = _32_channel_data[(8 + (32*4 +8)*boradNumber)*j + 7];
                        }
                    }



                    dataProcess->runProcess(data);
                    dataProcess->runFFT();
                    for(int channel = 0; channel < channel_number; ++channel){
                        if(channel_enable_map[channel]){
                            if(drawframe[channel]->hasDataToDraw(sampleRate, dataProcess->y_out[channel],
                                                                 channel%2 ? Qt::red : Qt::blue,
                                                                 dataProcess->min[channel],dataProcess->max[channel],dataProcess->rms[channel],
                                                                 mark,rms_w_count,rms_w_length,
                                                                 elect_off_p[channel],elect_off_n[channel])
                                    <0){
                                stop_m();
                                break;
                            }
                        }
                    }
                    drawFFT();
                    qApp->processEvents();
                    QTime timer = QTime::currentTime().addMSecs(100);
                    while( QTime::currentTime() < timer )
                        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
                }
            }
        }
        /**************   生成模拟正弦波并画图*******************/
        else{
            QMessageBox::StandardButton rb  = QMessageBox::question(this,"","是否使用模拟信号绘图");
            if(QMessageBox::No == rb){
                running = false;
                ui->runButton->setEnabled(true);
                ui->stopButton->setEnabled(false);
                return;
            }
            //产生10Hz递增,带100uV直流,幅度100uV正弦信号，采样率为1kHz
            for(int i = 0; i<channel_number; ++i){
                for(int j = 0; j< per_channel_number ;j++){
                    double y = 100.0*sin(2*PI_m*0.01*(i+1)*static_cast<double>(j))+100.0;
                    data[i][j] = y;
                }
            }
            for(int i = 0; i<per_channel_number; ++i){
                mark[i] = 0;
            }
            mark[static_cast<int>(per_channel_number/2)] = 1;

            while(running){
                dataProcess->runProcess(data);
                if(channel_number == 32)
                    dataProcess->runFFT();
                for(int channel = 0; channel < channel_number; ++channel){
                    if(channel_enable_map[channel]){
                        if(drawframe[channel]->hasDataToDraw(sampleRate, dataProcess->y_out[channel],
                                                             channel%2 ? Qt::red : Qt::blue,
                                                             dataProcess->min[channel],dataProcess->max[channel],dataProcess->rms[channel],
                                                             mark,rms_w_count,rms_w_length,
                                                             channel%2 ? true:false, false)<0)
                        {
                            stop_m();
                            break;
                        }
                    }
                }
                if(channel_number == 32)
                    drawFFT();
                qApp->processEvents();
                //int delay = (int)((49.0/500000.0)*(double)time_scale);
                //QTime timer = QTime::currentTime().addMSecs(100);
                //while( QTime::currentTime() < timer )
                //    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
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
                 if(channel_number == 32)
                     dataProcess->runFFT();
                 for(int channel = 0; channel<channel_number; ++channel){
                     if(channel_enable_map[channel]){
                         if(drawframe[channel]->hasDataToDraw(sampleRate, dataProcess->y_out[channel],
                                                              channel%2 ? Qt::red : Qt::blue ,
                                                              dataProcess->min[channel],dataProcess->max[channel],dataProcess->rms[channel],
                                                              mark,rms_w_count,rms_w_length,
                                                              elect_off_p[channel], elect_off_n[channel])
                                 <0){
                             stop_m();
                             break;
                         }

                     }
                 }
              }
              if(channel_number == 32)
                 drawFFT();
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
/***************  UI 槽函数***********
 *   启用命令调试接收返回值模式
 */
void MainWindow::changeCommandReturn(){
    if(Qt::Checked == ui->command_return_checkBox->checkState())
        tcp->commandReturn = true;
    else if(Qt::Unchecked == ui->command_return_checkBox->checkState())
        tcp->commandReturn = false;
}
/***************  UI 槽函数***********
 *   打开帮助说明窗口
 */
void MainWindow::showhelpmessage(){
    QObject* sender = QObject::sender();
    if("rms_help_pushButton" == sender->objectName()){
        QMessageBox::about(this,"Help Message","RMS: we show the data by windowsize = 100, overlap = 0 \n we draw the glazer by windowsize = 200, overlap = 30");
    }
    else if("path_help_pushButton" == sender->objectName()){
        QMessageBox::about(this,"Help Message","default save path is current filedir, and will set up a new file \"ZJUEEGDATA\"");
    }
    else if("command_help_pushButton" == sender->objectName()){
        QMessageBox::about(this,"Help Message","Configure the Register: AA 03 Addr Value\n Read the Register: AA 04 Addr");
    }
}
/***************  UI 槽函数***********
 *   打开通道控制窗口
 */
void MainWindow::openChannelSetWidget(){
    ChannelSet ch_set_window(checkbox,channel_number);
    ch_set_window.show();
    ch_set_window.exec();
    changeChannelEnable();
}
void MainWindow::changeChannelNumber(int index){
    switch(index){
        case 0: channel_number = 64;break;
        case 1: channel_number = 32;break;
        case 2: channel_number = 96;break;
        case 3: channel_number = 128;break;
        case 4: channel_number = 160;break;
        case 5: channel_number = 192;break;
        case 6: channel_number = 224;break;
        case 7: channel_number = 256;break;
    default:channel_number = 32;
    }
    /*
    //重新设置容器大小
    data.resize(channel_number);
    for(int i=0;i<channel_number;i++){
        data[i].resize(per_channel_number);
    }
    elect_off_p.resize(channel_number);
    elect_off_n.resize(channel_number);
    mark.resize(per_channel_number);
    //更新dataProcess
    delete dataProcess;
    dataProcess = new SignalProcess(channel_number,per_channel_number);
    //更新tcp
    delete tcp;
    tcp = new TcpConnect(ip,port,Tcp_server,channel_number);
    connect(this->tcp,SIGNAL(boardStart()),this,SLOT(connectToBardDone()));
    //更新FFT绘图
    initFFTWidget(true,fft_xmin,fft_xmax,fft_ymin,fft_ymax);
    */
    switch(channel_number){
    case 32: ui->tab_1->setEnabled(true);ui->tab_2->setEnabled(false);ui->tab_3->setEnabled(false);ui->tab_4->setEnabled(false);
        ui->tab_5->setEnabled(false);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);
        break;
    case 64: ui->tab_1->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(false);ui->tab_4->setEnabled(false);
        ui->tab_5->setEnabled(false);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);
        break;
    case 96: ui->tab_1->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(false);
        ui->tab_5->setEnabled(false);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);
        break;
    case 128: ui->tab_1->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(false);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);
        break;
    case 160: ui->tab_1->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(true);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);
        break;
    case 192: ui->tab_1->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(true);ui->tab_6->setEnabled(true);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);
        break;
    case 224: ui->tab_1->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(true);ui->tab_6->setEnabled(true);ui->tab_7->setEnabled(true);ui->tab_8->setEnabled(false);
        break;
    case 256: ui->tab_1->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(true);ui->tab_6->setEnabled(true);ui->tab_7->setEnabled(true);ui->tab_8->setEnabled(true);
        break;
    default: ui->tab_1->setEnabled(true);ui->tab_2->setEnabled(false);ui->tab_3->setEnabled(false);ui->tab_4->setEnabled(false);
        ui->tab_5->setEnabled(false);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);
    }
}
/***************  重写X按钮事件***********
 *   实现关闭程序时，如果还连接着板子，则给板子发送停止采集数据指令
 *   解决关闭程序时，如果绘制还未完成，后台程序未退出问题
 */
void MainWindow::closeEvent(QCloseEvent*){
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

/**** 打开FFT配置窗口 *****/
void MainWindow::openFFTSetWidget(){
    FFTSet fftsetWidget(fft_selfControl,fft_xmin,fft_xmax,fft_ymin,fft_ymax);
    connect(&fftsetWidget,SIGNAL(changeFFtWidget(bool,double,double,double,double)),this,SLOT(initFFTWidget(bool,double,double,double,double)));
    fftsetWidget.show();
    fftsetWidget.exec();
}

/**** 初始化FFT绘图窗口 *****/
void MainWindow::initFFTWidget(bool selfControl,double xmin,double xmax,double ymin,double ymax){
    fft_selfControl = selfControl;
    connect(ui->FFTWidget->xAxis,SIGNAL(rangeChanged(QCPRange)),ui->FFTWidget->xAxis2,SLOT(setRange(QCPRange)));
    connect(ui->FFTWidget->yAxis,SIGNAL(rangeChanged(QCPRange)),ui->FFTWidget->yAxis2,SLOT(setRange(QCPRange)));
    ui->FFTWidget->xAxis2->setVisible(true);
    ui->FFTWidget->xAxis2->setTickLabels(false);
    ui->FFTWidget->yAxis2->setVisible(true);
    ui->FFTWidget->yAxis2->setTickLabels(false);
    ui->FFTWidget->xAxis->setLabel("频率");
    ui->FFTWidget->yAxis->setLabel("幅度");
    if(!fft_selfControl){
        fft_ymax = ymax; fft_ymin = ymin;
        fft_xmax = xmax; fft_xmin = xmin;
        ui->FFTWidget->yAxis->setRange(fft_ymin,fft_ymax);
        ui->FFTWidget->xAxis->setRange(fft_xmin,fft_xmax);
    }
    ui->FFTWidget->setInteractions(QCP::iRangeDrag|QCP::iRangeZoom|QCP::iSelectPlottables);//缩放图形
    pen[0].setColor(QColor("red"));
    pen[1].setColor(QColor("green"));
    pen[2].setColor(QColor("blue"));
    pen[3].setColor(QColor("black"));
    pen[4].setColor(QColor("darkRed"));
    pen[5].setColor(QColor("darkGreen"));
    pen[6].setColor(QColor("darkBlue"));
    pen[7].setColor(QColor("Cyan"));
    pen[8].setColor(QColor("magenta"));
    pen[9].setColor(QColor("yellow"));
    pen[10].setColor(QColor("gray"));
    pen[11].setColor(QColor("darkCyan"));
    pen[12].setColor(QColor("darkMagenta"));
    pen[13].setColor(QColor("darkYellow"));
    pen[14].setColor(QColor("darkGray"));
    pen[15].setColor(QColor(188,143,11));
    pen[16].setColor(QColor(139,69,19));
    pen[17].setColor(QColor(160,82,45));
    pen[18].setColor(QColor(245,222,179));
    pen[19].setColor(QColor(244,164,96));
    pen[20].setColor(QColor(255,165,0));
    pen[21].setColor(QColor(255,140,0));
    pen[22].setColor(QColor(255,105,180));
    pen[23].setColor(QColor(255,20,147));
    pen[24].setColor(QColor(219,112,147));
    pen[25].setColor(QColor(176,48,96));
    pen[26].setColor(QColor(255,0,255));
    pen[27].setColor(QColor(160,32,240));
    pen[28].setColor(QColor(0,191,255));
    pen[29].setColor(QColor(0,178,238));
    pen[30].setColor(QColor(0,135,206,255));
    pen[31].setColor(QColor(104,34,139));
    for(int i =0; i<channel_number; ++i){
        ui->FFTWidget->addGraph();
        ui->FFTWidget->graph(i)->setPen(pen[i%32]);
    }
}
/**** FFT绘图 *****/
void MainWindow::drawFFT(){
    for(int i =0 ;i<channel_number; ++i){
        ui->FFTWidget->graph(i)->data().clear();
        if(channel_enable_map[i]){
            ui->FFTWidget->graph(i)->setVisible(true);
            ui->FFTWidget->graph(i)->setData(dataProcess->fft_x[i],dataProcess->fft_y[i]);
        }
        else
            ui->FFTWidget->graph(i)->setVisible(false);
    }
    if(fft_selfControl)
        ui->FFTWidget->rescaleAxes();
    ui->FFTWidget->replot();
}

void MainWindow::openSTFTWidget(){
    if(!stft_window)
        delete stft_window;
    stft_window = new STFT;
    stft_window->show();
}

