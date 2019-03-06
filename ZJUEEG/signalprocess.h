#ifndef SIGNALPROCESS_H
#define SIGNALPROCESS_H

#include <QVector>
#include "Iir.h"  //use external library from github

#define PI  3.14159265359

const int order = 14; // 4th order (=2 biquads)

class SignalProcess
{
public:
    SignalProcess(int ch_number,int perch_data_number);

    void runProcess(QVector<QVector <double>>& data_in);
    void setNotchFilter(double sampleRate, bool notchEnable, double notchFreq=50.0, double bandwidth=10.0);
    void setBandFilter(double sampleRate, bool bandEnable, double fl,double fh);
    void setHighPassFilter(bool enable);

    int channel_number;
    int per_channel_data_number;

    QVector<QVector<double>> y_out; //[channel_number][per_channel_data_number]
    QVector<double> min,max,rms;

private:
    bool notch_enable;
    bool band_enable;
    bool hpass_enable;
    double a1_notch,a2_notch,b0_notch,b1_notch,b2_notch;
    double a1_band, a2_band, b0_band, b1_band, b2_band;
    double a1_hpass, b0_hpass, b1_hpass;

    QVector<QVector<double>> x0_x1_notch;//[channel_number][2]
    QVector<QVector<double>> y0_y1_notch;//[channel_number][2]
    QVector<QVector<double>> x0_x1_band;
    QVector<QVector<double>> y0_y1_band;
    QVector<QVector<double>> y0_hpass;
    QVector<QVector<double>> x0_hpass;
    QVector<QVector<double>> y_notch; //[channel_number][per_channel_data_number]
    QVector<QVector<double>> y_band; //[channel_number][per_channel_data_number]
    QVector<QVector<double>> y_hpass; //[channel_number][per_channel_data_number]

    void copyData(QVector<QVector<double>>& data_in);//复制data_in到data_out
    void runBandFilter(QVector<QVector <double>>& data_in);
    void runNotchFilter(QVector<QVector <double>>& data_in);
    void runHighPassFilter(QVector<QVector <double>>& data_in);
    void calculateMinMax(QVector<QVector<double>>& data_in);//计算最大最小值
    void calculateRMS(QVector<QVector<double>>& data_in);//计算均方根值
    void allocateSpaceForVector2D(QVector<QVector<double>>& vector, int first_size,int second_size);
    void fillVectorDoubleZero(QVector<QVector<double>>& vector);

    //实例化滤波器，填充滤波器容器,这里按32处理。
    //注：不可使用容器存储实例化对象，否则出现SIGSEGV错误
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter0;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter1;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter2;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter3;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter4;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter5;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter6;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter7;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter8;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter9;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter10;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter11;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter12;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter13;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter14;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter15;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter16;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter17;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter18;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter19;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter20;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter21;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter22;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter23;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter24;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter25;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter26;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter27;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter28;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter29;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter30;
    Iir::Butterworth::BandPass<order,Iir::DirectFormII> bp_filter31;

};



#endif // SIGNALPROCESS_H
