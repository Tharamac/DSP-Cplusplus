#ifndef DSP_HPP_INCLUDED
#define DSP_HPP_INCLUDED
#endif // DSP_HPP_INCLUDED


#include "gnuplot_i.hpp"
#include <iostream>
#include <conio.h>
#include <complex>
#include <fftw3.h>
#include <fstream>
#include <math.h>
#include <vector>
#include <array>
#include <iterator>
#include <algorithm>
using namespace std;

#define Fs 44100
#define re 0
#define im 1

void hann(const vector<short> &data, short* output, int point_start, int window_size){
    //all this function argument are in sampling point
    for(int i = point_start; i < point_start + window_size ; i++){
      output[i - point_start] = data[i] * (0.5* (1 - cos(2*M_PI*i/(window_size))));
    }
}

void hann(const vector<double> &data, double *output, int point_start, int window_size){
    //all this function argument are in sampling point
    for(int i = point_start; i < point_start + window_size ; i++){
      output[i - point_start] = data[i] * (0.5* (1 - cos(2*M_PI*i/(window_size))));
    }
}


//from stack overflow
double geometric_mean(std::vector<double> const & data)
{
    long long ex = 0;
    auto do_bucket = [&data,&ex](int first,int last) -> double
    {
        double ans = 1.0;
        for ( ;first != last;++first)
        {
            int i;
            ans *= std::frexp(data[first],&i);
            ex+=i;
        }
        return ans;
    };

    const int bucket_size = -std::log2( std::numeric_limits<double>::min() );
    std::size_t buckets = data.size() / bucket_size;

    double invN = 1.0 / data.size();
    double m = 1.0;

    for (std::size_t i = 0;i < buckets;++i)
        m *= std::pow( do_bucket(i * bucket_size,(i+1) * bucket_size),invN );

    m*= std::pow( do_bucket( buckets * bucket_size, data.size() ),invN );

    return std::pow( std::numeric_limits<double>::radix,ex * invN ) * m;
}

double short_term_energy(double *frame, size_t N){
    double sum = 0;
    for(size_t i = 0; i < N; ++i){
        sum += frame[i]*frame[i];
    }
    return sum;
}

double zcr(double *input, size_t N){
    double sum = 0.0;
    for(size_t i = 0; i < N - 1 ; i++){
       sum += (input[i]*input[i+1] < 0 ) ? 1 : 0;
    }
    return sum / (N - 1);
}

double find_peak(vector<double> &real_spectrum, int N){
    int index = max_element(real_spectrum.begin(), real_spectrum.end()) - real_spectrum.begin();
    double freq = 0.5*index*Fs;
    freq /= (float)N;
    return freq;
}

void find5peaks(vector<double> real_spectrum, int N, vector<double>& topfreq){
    if (N < 5) {
        printf(" Invalid Input ");
        return;
    }
    double top5loud[5] = {0.0};


    for (int i = 1; i < (2000*N/44100) ; i ++) {
        if (real_spectrum[i] > top5loud[0]) {
            for(int j = 4; j > 0; j--){
                top5loud[j] = top5loud[j-1];
                topfreq[j] = topfreq[j-1];
            }
            top5loud[0] = real_spectrum[i];
            topfreq[0] = i*Fs*0.5/N;
        }else if (real_spectrum[i] > top5loud[1]) {
           for(int j = 4; j > 1; j--){
                top5loud[j] = top5loud[j-1];
                topfreq[j] = topfreq[j-1];
            }
            top5loud[1] = real_spectrum[i];
            topfreq[1] = i*Fs*0.5/N;
        }

        else if (real_spectrum[i]> top5loud[2]){
            for(int j = 4; j > 2; j--){
                top5loud[j] = top5loud[j-1];
                topfreq[j] = topfreq[j-1];
            }
            top5loud[2] = real_spectrum[i];
            topfreq[2] = i*Fs*0.5/N;

        }else if(real_spectrum[i]> top5loud[3]){
            for(int j = 4; j > 3; j--){
                top5loud[j] = top5loud[j-1];
                topfreq[j] = topfreq[j-1];
            }
            top5loud[3] = real_spectrum[i];
            topfreq[3] = i*Fs*0.5/N;
        }else if(real_spectrum[i]> top5loud[3]){
            top5loud[4] = top5loud[3];
            topfreq[4] = topfreq[3];
            top5loud[4] = real_spectrum[i];
            topfreq[4] = i*Fs*0.5/N;
        }


    }
}

double spectral_flatness(vector<double> real_spectrum,  size_t N){
    double arithmean = 0;
    double geomean = geometric_mean(real_spectrum);
    for(size_t i = 0; i < N; ++i){
        arithmean += real_spectrum[i];
    }
    arithmean /= (float)N;
    return 10*log10(geomean/arithmean);
}

int note_shift(double freq, double base_freq){
    return round(12*log2(freq/base_freq));
}


void plot(vector<double> x, string type, string title ){
    try
    {
        Gnuplot g1;
        g1.set_grid();
      //    g1 << "set format x \'%g\' " <<  "set logscale x";
        g1.set_style(type).plot_x(x,title);
        g1.showonscreen();
        getch();
    }
    catch (GnuplotException ge)
    {
        cout << ge.what() << endl;
    }
}
void plot(vector<double> x, vector<double> y, string type, string title){
    try
    {
        Gnuplot g1;
        g1.set_grid();

        g1.set_style(type).plot_xy(x,y,title);
        g1.showonscreen();
        getch();
    }
    catch (GnuplotException ge)
    {
        cout << ge.what() << endl;
    }
}

void VAD_convert(vector<int> vad, vector<int> new_vad){


}


double filterBank_with_peak(vector<double> real_spectrum, int note, double note_ref, int FFT_size){
	vector<double> temp(real_spectrum.size());
	for(size_t i = 0; i <  real_spectrum.size(); i++){
        temp[i] = 0;
	}

	//Gnuplot test;
	/*
	vector<double> sample(real_spectrum.size());
    for(int i = 0; i < sample.size() ; i++){
        sample[i] = 1;
    }
    */
	double start_freq = note_ref * pow(2 , (note - 1) / 12.0);
	double mid_freq = note_ref * pow(2 , note / 12.0);
	double end_freq = note_ref * pow(2 , (note + 1) / 12.0);
	//cout << start_freq << " " << mid_freq << " "  << end_freq << endl;
	const double log_slope = 12.0;

    for(size_t i = 1; i < real_spectrum.size() ; i++){
        double freq_i = i*0.5*Fs/FFT_size;
        if(freq_i < start_freq){
            temp[i] = 0;
           // cout << "-  " ;
        }else if (freq_i <= mid_freq){
            temp[i] =  log_slope * log2(freq_i/start_freq) * real_spectrum[i];
           // cout << "R  " ;
        }else if (freq_i <= end_freq){
            temp[i] =  log_slope * log2(end_freq/freq_i) * real_spectrum[i];
            //cout << "F  " ;
        }else{
            temp[i] = 0;
            //cout << "-  ";
        }
        //cout << freq_i << " " << temp[i] << endl;
    }
    double peak = *max_element(temp.begin(), temp.end());

/*
    test.set_grid();
    test.set_style("lines").plot_x(temp,"spectrum after applying filter");
    test.showonscreen();
    getch();
    */
    return peak;

}
