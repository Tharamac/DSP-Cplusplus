#define PI 3.14159265358979323846
#include "raw_audio.h"
#include "gnuplot_i.hpp"
#include "dsp.hpp"
#include "plot.hpp"
#include "musical_notes.h"
#include "chord.h"
#include "track_sequence.h"

#include <iostream>
#include <complex>
#include <fftw3.h>
#include <fstream>
#include <cmath>
#include <vector>
#include <array>
#include <iterator>
#include <algorithm>

#include "Chord-Detector-and-Chromagram\src\ChordDetector.h"
#include  "Chord-Detector-and-Chromagram\src\Chromagram.h"

#define Fs 44100
#define re 0
#define im 1
#define C3 130.81278265
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
    //RawAudio audio_input("samples/bew_do.pcm");
    RawAudio audio_input("samples/twinkle_samples.pcm");
    vector<double> doubledata = audio_input.doubledata;
    const int sample_size = audio_input.getSampleSize();

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
    cout << num_of_frame << endl;

    // VAD decision
    // False rejection optimization.

    const int energy_pthresh = 40;
    const int sfm_pthresh = 5;
    const double zcr_pthresh = 0.1;

    // Frame #0 - #28 assuming silence.
    double min_energy = *min_element(frame_energy.begin(), frame_energy.begin() + 28);
    double energy_threshold = energy_pthresh * min_energy;
    vector<bool> VADresult(num_of_frame);
    /* ignore on NDK */
    vector<int> energy_passed(num_of_frame);
    vector<int> freq_passed(num_of_frame);
    vector<int> sfm_passed(num_of_frame);
    vector<int> zcr_passed(num_of_frame);
    //double unvoice_count = 0;
    for(int i = 29; i < num_of_frame; ++i){
        int frame_score = 0;
        if(frame_energy[i] >= energy_threshold){
            energy_passed[i] = 1;
            frame_score++;
        }
        if(frame_zerocrossing_rate[i] <= zcr_pthresh){
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
/*
    vector<bool> VADcompare;
    unsigned int pos = 0;
    unsigned int k = 0;
    while (pos < VADresult.size()){
         bool result = (VADresult[pos] || VADresult[pos+1]) && VADresult[pos+2];
         VADcompare.push_back(result);
         pos += 2;
         k++;
    }
    VADcompare.resize(VADresult.size()-1);

*/


    // VAD plotting;
    /* ignore on NDK */
    //plot_data(frame_energy, frame_freq_peak, frame_spectralflatness, frame_zerocrossing_rate, VADresult);
    //plot_boolean(energy_passed, freq_passed, sfm_passed, zcr_passed, VADresult);

    // Note Detection
    // frame_size for note detection 0.08; (twice of VAD window size)
    // I use "nd" as a prefix on this part.
    const double nd_frame_size = 0.040;
    const int nd_sample_per_frame = nd_frame_size*Fs;
    const int N = 4096;
    const int N_fft = N/2+1;

    vector<double> nd_real_spectrum(N_fft);
    vector<double> nd_freq(N_fft);
    vector<vector<double>> nd_5freq_detect;
    vector<int> peak_offset;
    vector<int> notes_from_filter_bank;
    current_window_start = 0;
    fftw_complex* nd_spectrum = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (N_fft));
    int nd_num_of_frame = 0;

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

        double peak_of_each_notes[48];
        int note = 0;
        double max_peak = 0;
        for(int i = 0; i < 48; i++){
            peak_of_each_notes[i] = filterBank_with_peak(nd_real_spectrum, i, C3, N_fft);
            if(peak_of_each_notes[i] > max_peak){
                max_peak = peak_of_each_notes[i];
                note = i;
            }
        }
        notes_from_filter_bank.push_back(note);
        double peak = find_peak(nd_real_spectrum, N_fft);
        peak_offset.push_back(note_shift(peak,C3));
        nd_num_of_frame++;
        current_window_start += (nd_sample_per_frame/2);
    }while( current_window_start + nd_sample_per_frame <= sample_size );

    // store in 1D array, then convert to 2D array size[nd_num_of_frame][5]
   // nd_5freq_detect.resize(nd_num_of_frame);
  //find peak for note detection using chromagram
/*
    current_window_start = 0;
    vector<int> chroma_vector;
    int num_of_chromaframe = 0;
    do{
        vector<double> nd_chromaframe(nd_sample_per_frame);
        for(int i = 0; i < nd_sample_per_frame ; i++){
            nd_chromaframe[i] = doubledata[i + current_window_start];
        }

        Chromagram chromagram(nd_sample_per_frame,Fs);
        chromagram.setChromaCalculationInterval(nd_sample_per_frame);
        chromagram.processAudioFrame(nd_chromaframe);
        ChordDetector chordDetector;
        if (chromagram.isReady())
        {
            vector<double> chroma = chromagram.getChromagram();
            chordDetector.detectChord(chroma);
        }

        //cout << chordDetector.rootNote << "  " << chordDetector.quality << " | " << chordDetector.intervals << endl;
        int root_note = (int)chordDetector.rootNote;
        int chord_quality = (int)chordDetector.quality;
        int intervals = (int)chordDetector.intervals;
        ChordUnit chord(root_note, 4, chord_quality, intervals);
        if(VADresult[num_of_chromaframe] == 1)
            cout << chord.toString() << endl;
        else cout << "--" << endl;

        num_of_chromaframe++;
        current_window_start += (nd_sample_per_frame/2);
    }while( current_window_start + nd_sample_per_frame <= sample_size );
*/

    vector<NoteUnit> detected_notes_filter_bank;
    NoteUnit ref_note(NotePitch::C, 3);
    transformNotes(notes_from_filter_bank, detected_notes_filter_bank, VADresult, ref_note);
/*
    for(size_t i = 0; i < nd_num_of_frame; i++){
        cout << i << "." << detected_notes_filter_bank[i].toString() << endl;
    }
*/
    Melody track(detected_notes_filter_bank);
    track.calcKey();
    cout << track.toString() << endl;


    //  Test Note Detection
/*
    for(int i = 0 ; i < nd_num_of_frame ; i++){
       if(VADresult[i]){
            cout << nd_5freq_detect[i][0] << "\t";
            cout << note_shift(nd_5freq_detect[i][0], C3) << "\t";
            cout << notes_from_filter_bank[i] << "\t";

            //for(int j = 0; j < 5 ; j++)

            //cout << endl;
        }else
            cout << "-";
        cout << " | " << endl;
    }
    cout << endl;
*/
    //getch();
    fftw_free(spectrum);
    fftw_free(nd_spectrum);

    return 0;
}



