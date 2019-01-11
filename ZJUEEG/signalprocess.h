#ifndef SIGNALPROCESS_H
#define SIGNALPROCESS_H

#include<QVector>

#define PI  3.14159265359



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

};



#endif // SIGNALPROCESS_H
