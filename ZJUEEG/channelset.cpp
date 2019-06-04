/*
 *  通道配置窗口，集成自QDialog
 *  功能：全选通道、全不选通道，单选通道。注意，并没有向硬件发出通道开关的命令。只是影响时域以及频域通道绘图的可见性。
 *  通道数默认为32,修改通道数需要在 channelset.ui中增减控件，并修改该文件容器bu,
 */


#include "channelset.h"
#include "ui_channelset.h"

ChannelSet::ChannelSet(QVector<QCheckBox *> &checkbox, int c_Number,QWidget *parent) :
    QDialog(parent),//向父类递交参数
    ui(new Ui::ChannelSet),
    des_obj(checkbox),
    channelNumber(c_Number)
{
    ui->setupUi(this);

    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(changeChannelSet()));

    bu.resize(channelNumber);
    for(int i=0; i<channelNumber; ++i){
        QString obj_name_checkbox = "checkBox_"+QString::number(i+1);
        QCheckBox *p_checkbox = ui->tabWidget->findChild<QCheckBox *>(obj_name_checkbox);
        bu[i] = p_checkbox;
        bu[i]->setChecked(Qt::Checked == des_obj[i]->checkState());
    }

    switch(channelNumber){
    case 32: ui->tab->setEnabled(true);ui->tab_2->setEnabled(false);ui->tab_3->setEnabled(false);ui->tab_4->setEnabled(false);
        ui->tab_5->setEnabled(false);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);break;
    case 64: ui->tab->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(false);ui->tab_4->setEnabled(false);
        ui->tab_5->setEnabled(false);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);break;
    case 96: ui->tab->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(false);
        ui->tab_5->setEnabled(false);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);break;
    case 128: ui->tab->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(false);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);break;
    case 160: ui->tab->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(true);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);break;
    case 192: ui->tab->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(true);ui->tab_6->setEnabled(true);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);break;
    case 224: ui->tab->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(true);ui->tab_6->setEnabled(true);ui->tab_7->setEnabled(true);ui->tab_8->setEnabled(false);break;
    case 256: ui->tab->setEnabled(true);ui->tab_2->setEnabled(true);ui->tab_3->setEnabled(true);ui->tab_4->setEnabled(true);
        ui->tab_5->setEnabled(true);ui->tab_6->setEnabled(true);ui->tab_7->setEnabled(true);ui->tab_8->setEnabled(true);break;
    default: ui->tab->setEnabled(true);ui->tab_2->setEnabled(false);ui->tab_3->setEnabled(false);ui->tab_4->setEnabled(false);
        ui->tab_5->setEnabled(false);ui->tab_6->setEnabled(false);ui->tab_7->setEnabled(false);ui->tab_8->setEnabled(false);break;
    }

}

ChannelSet::~ChannelSet()
{
    delete ui;
}

void ChannelSet::chooseAll(){
    for(int i = 0; i<channelNumber;++i){
        bu[i]->setChecked(true);
    }
}
void ChannelSet::chooseNone(){
    for(int i = 0; i<channelNumber;++i){
        bu[i]->setChecked(false);
    }
}
void ChannelSet::changeChannelSet(){
    for(int i = 0; i<channelNumber;++i){
        des_obj[i]->setChecked(bu[i]->isChecked());
    }
}
