#define PI 3.14159265358979323846

#include "gnuplot_i.hpp"
#include "dsp.hpp"
#include "plot.hpp"
#include <iostream>
#include <complex>
#include <fftw3.h>
#include <fstream>
#include <cmath>
#include <vector>
#include <iterator>
#include <algorithm>

#include "ChordDetectorandChromagram/Chromagram.h"
#include "ChordDetectorandChromagram/ChordDetector.h"

#define Fs 44100
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
    // frame_size for VAD 0.04;
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


    //cout << num_of_frame << endl;
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
    //cout << "minimum energy = " << min_energy << endl;
    //cout << "minimum peak freq = " << min_freq << endl;
    //cout << "minimum sf = " << min_sfm << endl;

    double energy_threshold = energy_pthresh * min_energy;

    vector<int> VADresult(num_of_frame);
    /* ignore on NDK */
    vector<int> energy_passed(num_of_frame);
    vector<int> freq_passed(num_of_frame);
    vector<int> sfm_passed(num_of_frame);
    vector<int> zcr_passed(num_of_frame);
    double unvoice_count = 0;
    for(int i = 29; i < num_of_frame; ++i){
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

    // VAD plotting;
    /* ignore on NDK */
    //plot_data(frame_energy, frame_freq_peak, frame_spectralflatness, frame_zerocrossing_rate, VADresult);
    //plot_boolean(energy_passed, freq_passed, sfm_passed, zcr_passed, VADresult);

    // Note Detection
    // frame_size for note detection 0.08; (twice of VAD window size)
    // I use "nd" as a prefix on this part.
    const double nd_frame_size = 0.080;
    const int nd_sample_per_frame = nd_frame_size*Fs;
    const int N = 4096;
    const int N_fft = N/2+1;

    vector<double> nd_real_spectrum(N_fft);
    vector<double> nd_freq(N_fft);
    vector<double> nd_freq_detect;
    current_window_start = 0;
    fftw_complex* nd_spectrum = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (N_fft));
    int nd_num_of_frame;

    //find peak for note detection using peak frequency not accurate
    do{
        double nd_FFT_input[N] = {0.0};
        hann(doubledata, nd_FFT_input, current_window_start, nd_sample_per_frame);
        plan = fftw_plan_dft_r2c_1d(N, nd_FFT_input, nd_spectrum, FFTW_ESTIMATE);
        fftw_execute(plan);
        fftw_destroy_plan(plan);
        fftw_cleanup();
        for(int i = 0; i < N_fft; i++){
            nd_real_spectrum[i] = sqrt(nd_spectrum[re][i] * nd_spectrum[re][i] +  nd_spectrum[im][i] * nd_spectrum[im][i]);
            nd_freq[i] = (i/N)*Fs;
        }
        nd_freq_detect.push_back(find_peak(nd_real_spectrum, N_fft));
        nd_num_of_frame++;
        current_window_start += (nd_sample_per_frame);
    }while( current_window_start + nd_sample_per_frame <= sample_size );
    cout <<  nd_num_of_frame << endl;

    //find peak for note detection using chromagram
    current_window_start = 0;
    vector<int> chroma_vector;
    int num_of_chromaframe = 0;  cout <<  num_of_chromaframe << endl;
    do{
        double nd_chromaframe[nd_sample_per_frame] = {0.0};
        for(int i = 0; i < nd_sample_per_frame ; i++){
           nd_chromaframe[i] = doubledata[current_window_start];
        }
        Chromagram note_c (nd_sample_per_frame,Fs);
        note_c.setChromaCalculationInterval(nd_sample_per_frame);
        note_c.processAudioFrame(nd_chromaframe);

        if (note_c.isReady())
        {
            cout << "chroma frame" << num_of_chromaframe << endl;
            vector<double> chroma = note_c.getChromagram();
            for(int i = 0; i < chroma.size() ; i++){
                cout << i << ". " << chroma[i] << endl;
            }
            int max_note = max_element(chroma.begin(), chroma.end()) - chroma.begin();
            chroma_vector.push_back(max_note);
            // do something with the chromagram here
        }
        num_of_chromaframe++;
        current_window_start += (nd_sample_per_frame);
    }while( current_window_start + nd_sample_per_frame <= sample_size );

    cout <<  num_of_chromaframe << endl;
    //TODO:: Test & Compare freq on both VAD frame and Note Detection frame
    //TODO:: Android -> develop melody or chord choice, and Test argument passing to C++;

    for(int i = 0; i < num_of_chromaframe; i++){
       // if(VADresult[i*2] == 1)
            //cout << VADresult[i/2]
            cout << i << ". " << chroma_vector[i] << endl;
       // else{
      //      cout << i << ". -\n";
       // }
    }
    cout << endl;
    /*
    for(int i = 0; i < num_of_frame; i++){
        if(VADresult[i] == 1)
            cout << i << " " << frame_freq_peak[i] << " distance from A2 => " << note_shift(frame_freq_peak[i],110) << endl;
        else{
            cout << i << " ------------------------------\n";
        }
    }
    cout << endl;
    for(int i = 0; i < nd_num_of_frame; i++){
        if(VADresult[i*2] == 1)
            //cout << VADresult[i/2]
            cout << i << " " << nd_freq_detect[i] << " distance from A2 => " << note_shift(nd_freq_detect[i],110) << endl;
        else{
            cout << i << " ------------------------------\n";
        }
    }
*/


    //getch();
    delete[] bytedata;
    fftw_free(spectrum);
    fftw_free(nd_spectrum);

    return 0;
}



