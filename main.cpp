#define PI 3.14159265358979323846

#include "gnuplot_i.hpp"
#include "dsp.hpp"
#include <iostream>
#include <complex>
#include <fftw3.h>
#include <fstream>
#include <cmath>
#include <vector>
#include <iterator>
#include <algorithm>



#define Fs 44100;
#define re 0
#define im 1
using namespace std;




/*
    assume that sample_size = 8820
    0        data(0 -> 4409)            while ( 2205 + 4410 = 6615 < 8820 ) continue;
    2205     data(2205 -> 6614)         while ( 4410 + 4410 = 8820 <= 8820) continue;
    4410     data(4410 -> 8819)         while ( 6615 + 4410 = 11025 !< 8820 ) exit;
    6615     data exceed!

    assume that sample_size = 7540
    0        data(0 -> 4409)            while ( 2205 + 4410 = 6615 < 7540 ) continue;
    2205     data(2205 -> 6614)         while ( 4410 + 4410 = 8820 !< 7540) exit;
*/
//TODO:: change format form short into percentage
int main(){
    ifstream pcm;
    char* bytedata;
    int size;
   // pcm.open("voice16bit.pcm",ios::in|ios::binary|ios::ate);
    pcm.open("twinkle_samples.pcm",ios::in|ios::binary|ios::ate);
    if(pcm.is_open()){

        size = (int) pcm.tellg();
        cout << "size = " << size << " bytes" << endl;
        pcm.seekg (0, ios::beg);
        bytedata = new char[size];
        pcm.read(bytedata, size);
        pcm.close();
    }else cout << "Unable to open file" << endl;
    const int sample_size = size / 2;
    cout << "sample_size = " << sample_size << " bytes" << endl;
    vector<short> shortdata(sample_size);
    vector<double> doubledata(sample_size);


    //fftw_complex spectrum[sample_size];
    for(int i = 0; i < sample_size; i++){
        shortdata[i] = (( (short) bytedata[i*2+1] ) << 8 ) | (bytedata[i*2] & 0xFF);
        doubledata[i] = (double)shortdata[i] / 32768.0;
    }
    //jmp ROUTUNE_1

 //  for(int i = 54684; i < 55125; i++ ){
 //       printf("%d. %d , %f  \n", i ,shortdata[i] , doubledata[i]);
 //   }



    const double frame_size = 0.04;
    //frame_size for VAD 0.04;
    //frame_size for pitch track 0.075;
    const int sample_per_frame = frame_size*Fs;

    const int N = 4096;
    const int N_fft = N/2+1;

    vector<double> real_spectrum(N_fft);
    vector<double> freq(N_fft);

    int current_window_start = 0;
    vector<double> frame_energy;
    vector<double> frame_freq_peak;
    vector<double> frame_spectralflatness;
    vector<double> frame_zerocrossing_rate;
    int num_of_frame = 0;
    fftw_complex* spectrum = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));
    fftw_plan plan;
    do{
        double FFT_input[N] = {0.0};
        //apply hann window
        hann(doubledata, FFT_input, current_window_start, sample_per_frame) ;// 2205, 4410
        frame_energy.push_back(short_term_energy(FFT_input, N));
        frame_zerocrossing_rate.push_back(zcr(FFT_input, N));
        //
        plan = fftw_plan_dft_r2c_1d(4096, FFT_input, spectrum, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);
        fftw_cleanup();
        for(int i = 0; i <  N_fft; i++){
            real_spectrum[i] = sqrt(spectrum[re][i] * spectrum[re][i] +  spectrum[im][i] * spectrum[im][i]);
            freq[i] = (i/N)*Fs;
        }
        frame_freq_peak.push_back(find_peak(real_spectrum, N_fft));
        frame_spectralflatness.push_back(spectral_flatness(real_spectrum, N_fft));

        num_of_frame++;
      /*  if(num_of_frame == 192){
            vector<double> frame(begin(FFT_input), end(FFT_input));
           // for(int i = 0;i < N/2+1;i++)
           printf("energy. %f \n" ,frame_energy[191]);
            plot(real_spectrum,"impulses","Frame Data");
        }*/

        //cout << num_of_frame << ". Start: " << current_window_start << " End: " << current_window_start + sample_per_frame << endl;
        current_window_start += (sample_per_frame/2);
    }while( current_window_start + sample_per_frame <= sample_size );


    //VAD test
    Gnuplot VADplot;
    VADplot.cmd(" set multiplot layout 4, 1 title \"Multiplot VAD\" font \",14\" ");
   // VADplot.cmd(" set tmargin 2 ");
        //plot energy
    VADplot.cmd(" set title \"Frame Energy\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("impulses").plot_x(frame_energy,"Frame Energy");
        //plot max freq
    VADplot.cmd(" set title \"Frame Max Frequency\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("impulses").plot_x(frame_freq_peak,"Frame Max Freq");
        //plot SF
    VADplot.cmd(" set title \"Frame Spectral Flatness\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("impulses").plot_x(frame_spectralflatness,"Frame Energy");
        //plot zero crossing
    VADplot.cmd(" set title \"Frame Zero-Crossing Rate\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("impulses").plot_x(frame_zerocrossing_rate,"Frame Energy");
    VADplot.cmd(" unset multiplot ");
    VADplot.showonscreen();
    getch();
    /*
    // Note Detection
    do{
        double FFT_input[8192] = {0.0};
        //apply hann window
        hann(doubledata, FFT_input, current_window_start, sample_per_frame) ;// 2205, 4410
        current_window_start += (sample_per_frame/2);
        //
        plan = fftw_plan_dft_r2c_1d(8192, FFT_input, spectrum, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);
        fftw_cleanup();
        spectrum_process(spectrum, real_spectrum);
        num_of_frame++;
    }while( current_window_start + sample_per_frame <= sample_size );

    cout << i << endl;

*/
    delete[] bytedata;
    fftw_free(spectrum);

 /*   for(int i = 0; i < n ; i++){
        x[i][re] = i + 1;
        x[i][im] = 0;
    }
    fftw_plan plan = fftw_plan_dft_1d(n, x, y, FFTW_FORWARD, FFTW_ESTIMATE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);
    fftw_cleanup();
    cout << "FFT =" << endl;
    for(int i =0;i<n;i++){
        if(y[i][im] < 0)
            cout << y[i][re] << " - j" << abs(y[i][im]) << endl;
        else
            cout << y[i][re] << " + j" << y[i][im] << endl;
    }
*/
    return 0;
}



