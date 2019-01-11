#include "signalprocess.h"


SignalProcess::SignalProcess(int ch_number,int perch_data_number)
{
    notch_enable = false;
    band_enable = false;
    hpass_enable = false;
    a1_notch = 0; a2_notch = 0; b0_notch = 0; b1_notch = 0; b2_notch = 0;
    a1_band = 0; a2_band = 0; b0_band = 0; b1_band = 0; b2_band = 0;
    a1_hpass = -0.9993718787787191; b0_hpass = 1, b1_hpass = -1;

    channel_number = ch_number;
    per_channel_data_number = perch_data_number;

    //分配内存空间
    allocateSpaceForVector2D(x0_x1_notch, channel_number, 2);
    allocateSpaceForVector2D(y0_y1_notch, channel_number, 2);
    allocateSpaceForVector2D(y_notch, channel_number, per_channel_data_number);
    allocateSpaceForVector2D(x0_x1_band, channel_number, 2);
    allocateSpaceForVector2D(y0_y1_band, channel_number, 2);
    allocateSpaceForVector2D(y_band, channel_number, per_channel_data_number);
    allocateSpaceForVector2D(x0_hpass, channel_number, 1);
    allocateSpaceForVector2D(y0_hpass, channel_number, 1);
    allocateSpaceForVector2D(y_hpass, channel_number, per_channel_data_number);
    allocateSpaceForVector2D(y_out, channel_number, per_channel_data_number);
    min.resize(channel_number);
    max.resize(channel_number);
    rms.resize(channel_number);
    //存储x[0-1],x[0-2]和y[0-1]y[0-2]需要初始化为0
    fillVectorDoubleZero(x0_x1_notch);
    fillVectorDoubleZero(y0_y1_notch);
    fillVectorDoubleZero(x0_x1_band);
    fillVectorDoubleZero(y0_y1_band);
    fillVectorDoubleZero(x0_hpass);
    fillVectorDoubleZero(y0_hpass);
}

void SignalProcess::allocateSpaceForVector2D(QVector<QVector<double>>& vector, int first_size,int second_size)
{
    vector.resize(first_size);
    for (int i = 0; i<first_size; ++i){
        vector[i].resize(second_size);
    }
}

void SignalProcess::fillVectorDoubleZero(QVector<QVector<double>>& vector)
{
    int x_size = vector.size();
    if(x_size == 0)
        return;
    int y_size = vector[0].size();
    for(int i = 0; i<x_size; ++i){
        for(int j = 0 ;j<y_size;++j){
            vector[i][j]=0.0;
        }
    }
}

void SignalProcess::copyData(QVector<QVector<double>>& data_in){
    for(int channel = 0; channel<channel_number; ++channel){
        for(int i = 0; i < per_channel_data_number; ++i){
            y_out[channel][i]=data_in[channel][i];
        }
    }
}

/************ 信号处理  ********
 * 根据各个滤波器的使能进行滤波
 * 计算最小值，最大值，均方根值
 */
void SignalProcess::runProcess(QVector<QVector <double>>& data_in){
    if     (!hpass_enable && !notch_enable && !band_enable){
        copyData(data_in);
    }
    else if(!hpass_enable && !notch_enable &&  band_enable){
        runBandFilter(data_in);
        copyData(y_band);
    }
    else if(!hpass_enable &&  notch_enable && !band_enable){
        runNotchFilter(data_in);
        copyData(y_notch);
    }
    else if(!hpass_enable &&  notch_enable &&  band_enable){
        runNotchFilter(data_in);
        runBandFilter(y_notch);
        copyData(y_band);
    }
    else if( hpass_enable && !notch_enable && !band_enable){
        runHighPassFilter(data_in);
        copyData(y_hpass);
    }
    else if( hpass_enable && !notch_enable &&  band_enable){
        runHighPassFilter(data_in);
        runBandFilter(y_hpass);
        copyData(y_band);
    }
    else if( hpass_enable &&  notch_enable && !band_enable){
        runHighPassFilter(data_in);
        runNotchFilter(y_hpass);
        copyData(y_notch);
    }
    else {
        runHighPassFilter(data_in);
        runNotchFilter(y_hpass);
        runBandFilter(y_notch);
        copyData(y_band);
    }

    calculateMinMax(y_out);
    calculateRMS(y_out);
}

/************ 陷波  ********
 * 双二阶 IIR 滤波
 * y[n]=b0*x[n]+b1*x[n-1]+b2*x[n-2]-a1*y[n-1]-a2*y[n-2]
 */
void SignalProcess::setNotchFilter(double sampleRate, bool notchEnable,double notchFreq , double bandwidth)
{
    notch_enable = notchEnable;
    double d;
    d = exp(-PI * bandwidth / sampleRate);

    // Calculate biquad IIR filter coefficients.
    a1_notch = -(1.0 + d * d) * cos(2.0 * PI * notchFreq / sampleRate);
    a2_notch = d * d;
    b0_notch = (1 + d * d) / 2.0;
    b1_notch = a1_notch;
    b2_notch = b0_notch;
}
/*             x0_x1_notch 用于存储各通道计算y[0],y[1]时所需的x[0-1],x[0-2],即上一次滤波保存的最后两个值。
 *             y0_y1_notch 用于存储各通道计算y[0],y[1]时所需的y[0-1],y[0-2],即上一次滤波保存的最后两个值。
 */
void SignalProcess::runNotchFilter(QVector<QVector <double>>& data_in)
{  
        for(int channel = 0; channel<channel_number; ++channel){
            y_notch[channel][0] =
                    b2_notch * x0_x1_notch[channel][0] +
                    b1_notch * x0_x1_notch[channel][1] +
                    b0_notch * data_in[channel][0] -
                    a2_notch * y0_y1_notch[channel][0] -
                    a1_notch * y0_y1_notch[channel][1];
            y_notch[channel][1] =
                    b2_notch * x0_x1_notch[channel][1] +
                    b1_notch * data_in[channel][0]+
                    b0_notch * data_in[channel][1] -
                    a2_notch * y0_y1_notch[channel][1] -
                    a1_notch * y_notch[channel][0];
            for(int i = 2; i < per_channel_data_number; ++i){
                y_notch[channel][i] =
                        b2_notch * data_in[channel][i-2] +
                        b1_notch * data_in[channel][i-1]+
                        b0_notch * data_in[channel][i] -
                        a2_notch * y_notch[channel][i-2] -
                        a1_notch * y_notch[channel][i-1];
            }

            x0_x1_notch[channel][0] = data_in[channel][per_channel_data_number-2];//x[n-2]
            x0_x1_notch[channel][1] = data_in[channel][per_channel_data_number-1];//x[n-1]
            y0_y1_notch[channel][0] = y_notch[channel][per_channel_data_number-2];//y[n-2]
            y0_y1_notch[channel][1] = y_notch[channel][per_channel_data_number-1];//y[n-1]
        }
}


void SignalProcess::setBandFilter(double sampleRate, bool bandEnable, double fl,double fh)
{
    if(fl<fh)
        band_enable = bandEnable;
    else
        band_enable = false;

    double d;
    d = exp(-PI * (fh-fl) / sampleRate);

    // Calculate biquad IIR filter coefficients.

    a1_band = -(1.0 + d * d) * cos(2.0 * PI * ((fh+fl)/2.0) / sampleRate);
    a2_band = d * d;
    b0_band = (1 - d * d) / 2.0;
    //b0_band = (1.0 + d * d) * sin(2.0 * PI * ((fh+fl)/2.0) / sampleRate)/4;
    b1_band = 0;
    b2_band = -b0_band;

    /*
    b0_band = 0.243255495981982;
    b1_band = 0;
    b2_band = -b0_band;
    a1_band = -1.510402314455435;
    a2_band = 0.513489008036035;
    */
}

void SignalProcess::runBandFilter(QVector<QVector <double>>& data_in)
{
        for(int channel = 0; channel<channel_number; ++channel){
            y_band[channel][0] =
                    b2_band * x0_x1_band[channel][0] +
                    b1_band * x0_x1_band[channel][1] +
                    b0_band * data_in[channel][0] -
                    a2_band * y0_y1_band[channel][0] -
                    a1_band * y0_y1_band[channel][1];
            y_band[channel][1] =
                    b2_band * x0_x1_band[channel][1] +
                    b1_band * data_in[channel][0]+
                    b0_band * data_in[channel][1] -
                    a2_band * y0_y1_band[channel][1] -
                    a1_band * y_band[channel][0];
            for(int i = 2; i < per_channel_data_number; ++i){
                y_band[channel][i] =
                        b2_band * data_in[channel][i-2] +
                        b1_band * data_in[channel][i-1]+
                        b0_band * data_in[channel][i] -
                        a2_band * y_band[channel][i-2] -
                        a1_band * y_band[channel][i-1];
            }

            x0_x1_band[channel][0] = data_in[channel][per_channel_data_number-2];//x[n-2]
            x0_x1_band[channel][1] = data_in[channel][per_channel_data_number-1];//x[n-1]
            y0_y1_band[channel][0] = y_band[channel][per_channel_data_number-2];//y[n-2]
            y0_y1_band[channel][1] = y_band[channel][per_channel_data_number-1];//y[n-1]
        }
}

/************ 高通滤波  ********
 * 一阶 IIR 滤波
 * y[n]=b0*x[n]+b1*x[n-1]-a1*y[n-1]
 */
void SignalProcess::setHighPassFilter(bool enable){
    hpass_enable = enable;
    /* 1khz采样率，0.1截止频率参数：
     * a1_hpass = -0.9993718787787191; b0_hpass = 1, b1_hpass = -1;
     */
    //aHpf = exp(-1.0 * TWO_PI * cutoffFreq / sampleFreq);
    //bHpf = 1.0 - aHpf;
}
/************ 高通滤波  ********
 * 一阶 IIR 滤波
 * y[n]=b0*x[n]+b1*x[n-1]-a1*y[n-1]
 */
void SignalProcess::runHighPassFilter(QVector<QVector <double>>& data_in)
{
        for(int channel = 0; channel<channel_number; ++channel){
            y_hpass[channel][0] =
                b1_hpass * x0_hpass[channel][0] +
                b0_hpass * data_in[channel][0] -
                a1_hpass * y0_hpass[channel][0];
            for(int i = 1; i < per_channel_data_number; ++i){
                y_hpass[channel][i] =
                    b1_hpass * data_in[channel][i-1] +
                    b0_hpass * data_in[channel][i] -
                    a1_hpass * y_hpass[channel][i-1];
            }
            x0_hpass[channel][0] = data_in[channel][per_channel_data_number-1];
            y0_hpass[channel][0] = y_hpass[channel][per_channel_data_number-1];
        }
}

void SignalProcess::calculateMinMax(QVector<QVector<double>>& data_in){
    for(int channel =0; channel<channel_number; ++channel){
        min[channel]  = data_in[channel][0];
        max[channel]  = data_in[channel][0];
        for(int i = 1; i<per_channel_data_number; ++i){
            if(data_in[channel][i]<min[channel])
                min[channel] = data_in[channel][i];
            if(data_in[channel][i]>max[channel]){
                max[channel] = data_in[channel][i];
            }
        }
    }
}

void SignalProcess::calculateRMS(QVector<QVector<double>>& data_in){
    for(int channel =0; channel<channel_number; ++channel){
        double result = 0;
        for(int i = 0; i <per_channel_data_number; ++i){
            result += data_in[channel][i]*data_in[channel][i];
        }
        rms[channel] = sqrt(result/per_channel_data_number);
    }
}
