#ifndef SIGNALPROCESS_H
#define SIGNALPROCESS_H

/*数据处理部分
 * 包含滤波器：1）高通滤波器 2）带通滤波器 3）陷波器 4）希尔伯特滤波
 * 包含数据处理 1）最大最小值计算 2）均方根RMS计算 3）平均均值（RMS）计算
 * 滤波算法使用外部库，实现高阶滤波
*/


#include <QVector>
#include "Iir.h"  //use external library from github
#include "envolope/envolope.h"

#define PI  3.14159265359

const int order = 14; // 4th order (=2 biquads)

class SignalProcess
{
public:
    SignalProcess(int ch_number,int perch_data_number);

    void runProcess(QVector<QVector <double>>& data_in);
    void setNotchFilter(double sampleRate, bool notchEnable, double notchFreq=50.0, double bandwidth=20.0);
    void setBandFilter(double sampleRate, bool bandEnable, double fl,double fh);
    void setHighPassFilter(bool enable);
    void setHilbert(bool enable);
    void setArv(bool enable);
    int getArvWindowLength(void);
    int getArvWindowLengthOver(void);

    int channel_number;
    int per_channel_data_number;
    int arv_length;//窗长
    int arv_w_overlength;//重叠窗长

    QVector<QVector<double>> y_out; //[channel_number][per_channel_data_number]
    QVector<double> min,max,rms;

private:
    bool notch_enable;
    bool band_enable;
    bool hpass_enable;
    bool hilbert_enable;
    bool arv_enable;
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
    QVector<QVector<double>> y_hilbert;//[channel_number][per_channel_data_number]

    void copyData(QVector<QVector<double>>& data_in);//复制data_in到data_out
    void runBandFilter(QVector<QVector <double>>& data_in);
    void runNotchFilter(QVector<QVector <double>>& data_in);
    void runHighPassFilter(QVector<QVector <double>>& data_in);
    void runHilbert(QVector<QVector<double>>& data_in);
    void runArv(QVector<QVector<double>>& data_in);
    void calculateMinMax(QVector<QVector<double>>& data_in);//计算最大最小值
    void calculateRMS(QVector<QVector<double>>& data_in);//计算均方根值
    void allocateSpaceForVector2D(QVector<QVector<double>>& vector, int first_size,int second_size);
    void fillVectorDoubleZero(QVector<QVector<double>>& vector);

    //实例化滤波器，填充滤波器容器,这里按32处理。
    //注：不可使用容器存储实例化对象，否则出现SIGSEGV错误
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter0;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter1;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter2;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter3;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter4;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter5;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter6;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter7;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter8;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter9;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter10;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter11;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter12;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter13;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter14;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter15;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter16;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter17;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter18;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter19;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter20;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter21;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter22;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter23;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter24;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter25;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter26;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter27;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter28;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter29;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter30;
    Iir::ChebyshevI::BandPass<order,Iir::DirectFormII> bp_filter31;

    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter0;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter1;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter2;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter3;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter4;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter5;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter6;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter7;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter8;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter9;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter10;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter11;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter12;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter13;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter14;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter15;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter16;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter17;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter18;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter19;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter20;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter21;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter22;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter23;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter24;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter25;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter26;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter27;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter28;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter29;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter30;
    Iir::ChebyshevI::BandStop<order,Iir::DirectFormII> bs_filter31;
};



#endif // SIGNALPROCESS_H
