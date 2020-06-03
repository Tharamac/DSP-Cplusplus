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

    for(int i = 0; i < sample_size; i++){
        shortdata[i] = (( (short) bytedata[i*2+1] ) << 8 ) | (bytedata[i*2] & 0xFF);
        doubledata[i] = (double)shortdata[i] / 32768.0;
    }

    // VAD
    //frame_size for VAD 0.04;
    //frame_size for note detection 0.075;
    const double frame_size = 0.04;
    const int sample_per_frame = frame_size*Fs;
    const int vad_N = 2048;
    const int vad_N_fft = vad_N/2+1;

    vector<double> real_spectrum(vad_N_fft);
    vector<double> freq(vad_N_fft);

    int current_window_start = 0;
    vector<double> frame_energy;
    vector<double> frame_freq_peak;
    vector<double> frame_spectralflatness;
    vector<double> frame_zerocrossing_rate;

    int num_of_frame = 0;
    fftw_complex* spectrum = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (vad_N_fft));
    fftw_plan plan;
    //TODO:: False rejection optimization.
    do{
        double FFT_input[vad_N] = {0.0};
        hann(doubledata, FFT_input, current_window_start, sample_per_frame);
        frame_energy.push_back(short_term_energy(FFT_input, vad_N));
        frame_zerocrossing_rate.push_back(zcr(FFT_input, vad_N));
        plan = fftw_plan_dft_r2c_1d(vad_N, FFT_input, spectrum, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);
        fftw_cleanup();
        for(int i = 0; i <  vad_N_fft; i++){
            real_spectrum[i] = sqrt(spectrum[re][i] * spectrum[re][i] +  spectrum[im][i] * spectrum[im][i]);
            freq[i] = (i/vad_N)*Fs;

        }
        frame_freq_peak.push_back(find_peak(real_spectrum, vad_N_fft));
        frame_spectralflatness.push_back(abs(spectral_flatness(real_spectrum, vad_N_fft)));
        num_of_frame++;
        //cout << num_of_frame << ". Start: " << current_window_start << " End: " << current_window_start + sample_per_frame << endl;
        current_window_start += (sample_per_frame/2);
    }while( current_window_start + sample_per_frame <= sample_size );


    cout << num_of_frame << endl;
    // VAD decision
    // False rejection optimization.

    int frame_score = 0;

    const int energy_pthresh = 40;
    const int freq_pthresh = 0;
    const int sfm_pthresh = 5;
    const double zcr_pthresh = 0.1;



    // Frame #0 - #28 assuming silence.
    double min_energy = *min_element(frame_energy.begin(), frame_energy.begin() + 28);
    double min_freq = *min_element(frame_freq_peak.begin(), frame_freq_peak.begin() + 28 );
    double min_sfm = *min_element(frame_spectralflatness.begin(), frame_spectralflatness.begin() + 28);



    cout << "minimum energy = " << min_energy << endl;
    cout << "minimum peak freq = " << min_freq << endl;
    cout << "minimum sf = " << min_sfm << endl;

    double energy_threshold = energy_pthresh * min_energy;

    vector<int> VADresult(num_of_frame);
    /**/
    vector<int> energy_passed(num_of_frame);
    vector<int> freq_passed(num_of_frame);
    vector<int> sfm_passed(num_of_frame);
    vector<int> zcr_passed(num_of_frame);
    cout << "frame_energy" << endl;
    double unvoice_count = 0;
    for(int i = 29; i < num_of_frame; ++i){

        if(i > 520 && i < 550 ){
          cout << i << ". " << frame_energy[i] << " ? "  << energy_threshold << endl;
        }
        int frame_score = 0;
        if(frame_energy[i] >= energy_threshold){
            energy_passed[i] = 1;
            frame_score++;
        }
        if(frame_zerocrossing_rate[i] <= 0.1){
            zcr_passed[i] = 1;
            frame_score++;
        }
        if(frame_spectralflatness[i] > sfm_pthresh){
            sfm_passed[i] = 1;
            frame_score++;
        }
        if(frame_freq_peak[i] >= 220 && frame_freq_peak[i] <= 2000){
            freq_passed[i] = 1;
            frame_score++;
        }
        if(frame_score > 2){
            VADresult[i] = 1;
        }
    }


    //VAD test
    Gnuplot VADplot;
    //VADplot.cmd(" set multiplot layout 5, 1 title \"Multiplot VAD\" font \",14\" ");
   // VADplot.cmd(" set tmargin 2 ");
        //plot energy
    VADplot.cmd(" set title \"Frame Energy\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("lines").plot_x(frame_energy,"Frame Energy");
     /*   //plot max freq
    VADplot.cmd(" set title \"Frame Max Frequency\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("lines").plot_x(frame_freq_peak,"Frame Max Freq");
        //plot SF
    VADplot.cmd(" set title \"Frame Spectral Flatness\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("lines").plot_x(frame_spectralflatness,"Frame SFM");
        //plot zero crossing
    VADplot.cmd(" set title \"Frame Zero-Crossing Rate\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("lines").plot_x(frame_zerocrossing_rate,"Frame ZCR");
        //plotVAD Result
    VADplot.cmd(" set title \"Frame VAD\" ");
    VADplot.cmd(" unset key ");
    VADplot.set_grid();
    VADplot.set_style("impulses").plot_x(VADresult,"VAD result");
    VADplot.cmd(" unset multiplot ");
*/
    VADplot.showonscreen();

    // plot boolean of feature
     Gnuplot VADplot_bool;
    VADplot_bool.cmd(" set multiplot layout 5, 1 title \"Multiplot VAD bool\" font \",14\" ");
        //plot energy
    VADplot_bool.cmd(" set title \"Frame Energy\" ");
    VADplot_bool.cmd(" unset key ");
    VADplot_bool.set_grid();
    VADplot_bool.set_style("lines").plot_x(energy_passed,"Frame Energy");
        //plot max freq
    VADplot_bool.cmd(" set title \"Frame Max Frequency\" ");
    VADplot_bool.cmd(" unset key ");
    VADplot_bool.set_grid();
    VADplot_bool.set_style("lines").plot_x(freq_passed,"Frame Max Freq");
        //plot SF
    VADplot_bool.cmd(" set title \"Frame Spectral Flatness\" ");
    VADplot_bool.cmd(" unset key ");
    VADplot_bool.set_grid();
    VADplot_bool.set_style("lines").plot_x(sfm_passed,"Frame SFM");
        //plot zero crossing
    VADplot_bool.cmd(" set title \"Frame Zero-Crossing Rate\" ");
    VADplot_bool.cmd(" unset key ");
    VADplot_bool.set_grid();
    VADplot_bool.set_style("lines").plot_x(zcr_passed,"Frame ZCR");
        //plotVAD Result
    VADplot_bool.cmd(" set title \"Frame VAD\" ");
    VADplot_bool.cmd(" unset key ");
    VADplot_bool.set_grid();
    VADplot_bool.set_style("lines").plot_x(VADresult,"VAD result");
    VADplot_bool.cmd(" unset multiplot ");

    VADplot_bool.showonscreen();
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



