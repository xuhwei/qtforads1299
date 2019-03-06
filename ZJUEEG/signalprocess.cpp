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
/************ 带通  ********
 *
 *
 */
void SignalProcess::setBandFilter(double sampleRate, bool bandEnable, double fl,double fh)
{  
    if(fl<fh)
        band_enable = bandEnable;
    else
        band_enable = false;
    if(band_enable){
        double center_frequency = (fh+fl)/2.0; // Hz
        double width_freuency = fh-fl;//Hz
        bp_filter0.setup(sampleRate,center_frequency,width_freuency);
        bp_filter1.setup(sampleRate,center_frequency,width_freuency);
        bp_filter2.setup(sampleRate,center_frequency,width_freuency);
        bp_filter3.setup(sampleRate,center_frequency,width_freuency);
        bp_filter4.setup(sampleRate,center_frequency,width_freuency);
        bp_filter5.setup(sampleRate,center_frequency,width_freuency);
        bp_filter6.setup(sampleRate,center_frequency,width_freuency);
        bp_filter7.setup(sampleRate,center_frequency,width_freuency);
        bp_filter8.setup(sampleRate,center_frequency,width_freuency);
        bp_filter9.setup(sampleRate,center_frequency,width_freuency);
        bp_filter10.setup(sampleRate,center_frequency,width_freuency);
        bp_filter11.setup(sampleRate,center_frequency,width_freuency);
        bp_filter12.setup(sampleRate,center_frequency,width_freuency);
        bp_filter13.setup(sampleRate,center_frequency,width_freuency);
        bp_filter14.setup(sampleRate,center_frequency,width_freuency);
        bp_filter15.setup(sampleRate,center_frequency,width_freuency);
        bp_filter16.setup(sampleRate,center_frequency,width_freuency);
        bp_filter17.setup(sampleRate,center_frequency,width_freuency);
        bp_filter18.setup(sampleRate,center_frequency,width_freuency);
        bp_filter19.setup(sampleRate,center_frequency,width_freuency);
        bp_filter20.setup(sampleRate,center_frequency,width_freuency);
        bp_filter21.setup(sampleRate,center_frequency,width_freuency);
        bp_filter22.setup(sampleRate,center_frequency,width_freuency);
        bp_filter23.setup(sampleRate,center_frequency,width_freuency);
        bp_filter24.setup(sampleRate,center_frequency,width_freuency);
        bp_filter25.setup(sampleRate,center_frequency,width_freuency);
        bp_filter26.setup(sampleRate,center_frequency,width_freuency);
        bp_filter27.setup(sampleRate,center_frequency,width_freuency);
        bp_filter28.setup(sampleRate,center_frequency,width_freuency);
        bp_filter29.setup(sampleRate,center_frequency,width_freuency);
        bp_filter30.setup(sampleRate,center_frequency,width_freuency);
        bp_filter31.setup(sampleRate,center_frequency,width_freuency);
    }
}
void SignalProcess::runBandFilter(QVector<QVector <double>>& data_in)
{
    for(int i=0; i<per_channel_data_number; ++i){
        y_band[0][i] = bp_filter0.filter(data_in[0][i]);
        y_band[1][i] = bp_filter1.filter(data_in[1][i]);
        y_band[2][i] = bp_filter2.filter(data_in[2][i]);
        y_band[3][i] = bp_filter3.filter(data_in[3][i]);
        y_band[4][i] = bp_filter4.filter(data_in[4][i]);
        y_band[5][i] = bp_filter5.filter(data_in[5][i]);
        y_band[6][i] = bp_filter6.filter(data_in[6][i]);
        y_band[7][i] = bp_filter7.filter(data_in[7][i]);
        y_band[8][i] = bp_filter8.filter(data_in[8][i]);
        y_band[9][i] = bp_filter9.filter(data_in[9][i]);
        y_band[10][i] = bp_filter10.filter(data_in[10][i]);
        y_band[11][i] = bp_filter11.filter(data_in[11][i]);
        y_band[12][i] = bp_filter12.filter(data_in[12][i]);
        y_band[13][i] = bp_filter13.filter(data_in[13][i]);
        y_band[14][i] = bp_filter14.filter(data_in[14][i]);
        y_band[15][i] = bp_filter15.filter(data_in[15][i]);
        y_band[16][i] = bp_filter16.filter(data_in[16][i]);
        y_band[17][i] = bp_filter17.filter(data_in[17][i]);
        y_band[18][i] = bp_filter18.filter(data_in[18][i]);
        y_band[19][i] = bp_filter19.filter(data_in[19][i]);
        y_band[20][i] = bp_filter20.filter(data_in[20][i]);
        y_band[21][i] = bp_filter21.filter(data_in[21][i]);
        y_band[22][i] = bp_filter22.filter(data_in[22][i]);
        y_band[23][i] = bp_filter23.filter(data_in[23][i]);
        y_band[24][i] = bp_filter24.filter(data_in[24][i]);
        y_band[25][i] = bp_filter25.filter(data_in[25][i]);
        y_band[26][i] = bp_filter26.filter(data_in[26][i]);
        y_band[27][i] = bp_filter27.filter(data_in[27][i]);
        y_band[28][i] = bp_filter28.filter(data_in[28][i]);
        y_band[29][i] = bp_filter29.filter(data_in[29][i]);
        y_band[30][i] = bp_filter30.filter(data_in[30][i]);
        y_band[31][i] = bp_filter31.filter(data_in[31][i]);
    }
}
/*
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


    //b0_band = 0.243255495981982;
    //b1_band = 0;
    //b2_band = -b0_band;
    //a1_band = -1.510402314455435;
    //a2_band = 0.513489008036035;

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
*/

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
