#ifndef RAW_AUDIO_H_INCLUDED
#define RAW_AUDIO_H_INCLUDED
#include <fstream>
#include <vector>
#include <iostream>
using namespace std;

class RawAudio{

private:
    char *bytedata;
    int sample_size;

public:
    vector<short> shortdata;
    vector<double> doubledata;
    RawAudio(const char *path);
    ~RawAudio();
    int getSampleSize(){
        return sample_size;
    };
};



#endif // RAW_AUDIO_H_INCLUDED
