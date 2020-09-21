#include "raw_audio.h"

RawAudio::RawAudio(const char *path){
    ifstream pcm;
    int size;
    pcm.open(path, ios::in|ios::binary|ios::ate);
    cout << path << endl;
    if(pcm.is_open()){
        size = (int) pcm.tellg();
        pcm.seekg (0, ios::beg);
        bytedata = new char[size];
        pcm.read(bytedata, size);
        pcm.close();
    }else cout << "Unable to open file" << endl;
    sample_size = size/2;
    shortdata.resize(sample_size);
    doubledata.resize(sample_size);
    for(int i = 0; i < sample_size; i++){
        shortdata[i] = (((short) bytedata[i*2+1] ) << 8 ) | (bytedata[i*2] & 0xFF);
        doubledata[i] = (double)shortdata[i] / 32768.0;
    }
}

RawAudio::~RawAudio(){
    delete[] bytedata;
}
