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
#include <iterator>
#include <algorithm>
using namespace std;

#define Fs 44100;
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
    for(int i = 0; i < N; ++i){
        sum += frame[i]*frame[i];
    }
    return sum;
}

double zcr(double *input, size_t N){
    double sum = 0.0;
    for(int i = 0; i < N - 1 ; i++){
       sum += (input[i]*input[i+1] < 0 ) ? 1 : 0;
    }
    return sum / (N - 1);
}

double find_peak(vector<double> real_spectrum, int N){
    int index = max_element(real_spectrum.begin(), real_spectrum.end()) - real_spectrum.begin();
    double freq = 0.5*index*Fs;
    freq /= (float)N;
    return freq;
}

double spectral_flatness(vector<double> real_spectrum,  size_t N){
    double arithmean = 0;
    double geomean = geometric_mean(real_spectrum);
    for(int i = 0; i < N; ++i){
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




