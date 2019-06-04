#ifndef SIGNALPROCESS_H
#define SIGNALPROCESS_H

#include <QVector>
#include "Iir.h"  //use external library from github
#include "FFT/FFT.h"

#define PI_m  3.141592653589793

const int order = 8; // 8th order (=16 biquads)

class SignalProcess
{
public:
    SignalProcess(int ch_number,int perch_data_number);
    ~SignalProcess();

    void runProcess(QVector<QVector <double>>& data_in);
    void setNotchFilter(double sampleRate, bool notchEnable, double notchFreq=50.0, double bandwidth=10.0);
    void setBandFilter(double sampleRate, bool bandEnable, double fl,double fh);
    void setHighPassFilter(double sampleRate, bool enable);
    void setRMS(bool enable);
    int getRMSWindowLength(void);
    int getRMSWindowLengthOver(void);
    void runFFT();

    int rms_length;//窗长
    int rms_w_overlength;//重叠窗长

    QVector<QVector<double>> y_out; //[channel_number][per_channel_data_number]
    QVector<double> min,max,rms;
    emxArray_real_T * fftout;
    QVector<QVector<double>> fft_x;
    QVector<QVector<double>> fft_y;

private:
    const int FFT_N;
    int channel_number;
    int per_channel_data_number;
    bool notch_enable;
    bool band_enable;
    bool hpass_enable;
    bool rms_enable;
    int count_y_in_dataForFFT;
    emxArray_real_T* fftin;

    QVector<QVector<double>> y_notch; //[channel_number][per_channel_data_number]
    QVector<QVector<double>> y_band; //[channel_number][per_channel_data_number]
    QVector<QVector<double>> y_hpass; //[channel_number][per_channel_data_number]
    QVector<QVector<double>> y_in_dataForFFT;

    void copyData(QVector<QVector<double>>& data_in);//复制data_in到y_out
    void runBandFilter(QVector<QVector <double>>& data_in);
    void runNotchFilter(QVector<QVector <double>>& data_in);
    void runHighPassFilter(QVector<QVector <double>>& data_in);
    void runRMS(QVector<QVector<double>>& data_in);
    void calculateMinMax(QVector<QVector<double>>& data_in);//计算最大最小值
    void calculateRMS(QVector<QVector<double>>& data_in);//计算均方根值
    void allocateSpaceForVector2D(QVector<QVector<double>>& vector, int first_size,int second_size);
    void fillVectorDoubleZero(QVector<QVector<double>>& vector);

    //滤波器指针
    QVector<Iir::ChebyshevI::BandPass<order,Iir::DirectFormII>*> bp_filter;
    QVector<Iir::ChebyshevI::BandStop<order,Iir::DirectFormII>*> bs_filter;
    QVector<Iir::Butterworth::HighPass<2,Iir::DirectFormII>*> hp_filter;
};



#endif // SIGNALPROCESS_H
