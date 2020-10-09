#include "track_sequence.h"
Melody::Melody(vector<NoteUnit> &raw_note){
    //set start frame
    this->numOfFrame = raw_note.size();
    this->frameStart = -1;
    for(size_t i = 0; (raw_note[i].getPitch() == NotePitch::rest) && (i < this->numOfFrame); i++){
        this->frameStart = i;
    }
    this->frameStart++;
    if(frameStart == this->numOfFrame){
        cout << "Could not find start frame because there are not any note found." << endl;
        return;
    }
    //Iterate whole array to register note in Melody
    Note initNote(raw_note[frameStart].getPitch(),raw_note[frameStart].getOctave());
    this->noteSequence.push_back(initNote);
    for(size_t i = frameStart + 1; i < this->numOfFrame; ++i){
        Note* latestNote = &this->noteSequence[this->noteSequence.size() - 1];
        if(latestNote->equal(raw_note[i])){
            latestNote->increaseFrame();
        }else{
            Note temp(raw_note[i].getPitch(),raw_note[i].getOctave());
            this->noteSequence.push_back(temp);
            updateSequence();
        }
    }
    cleanTrack();
}

void Melody::updateSequence(){
    auto currentSize = this->noteSequence.size();
    Note* latestCompleteNote = &this->noteSequence[currentSize - 2];
   // Note* checkedCompleteNote = (currentSize < 3) ? &this->noteSequence[this->noteSequence.size() - 3] : NULL;

    if(latestCompleteNote->getFrameLength() <= 3){
        if(currentSize < 3){
            latestCompleteNote->makeRestNote();
        }else{
            Note* checkedCompleteNote = &this->noteSequence[this->noteSequence.size() - 3];
            checkedCompleteNote->increaseFrame(latestCompleteNote->getFrameLength());
            this->noteSequence.erase(this->noteSequence.begin() + (currentSize - 2));
        }
    }

}

bool pred(NoteUnit &a , NoteUnit &b){
    return a.equal(b);
}

void Melody::cleanTrack(){
    if(this->noteSequence[0].getPitch() == NotePitch::rest){
        this->noteSequence.erase(this->noteSequence.begin());
    }
    auto it = adjacent_find(this->noteSequence.begin(), this->noteSequence.end(), pred);
    while(it != this->noteSequence.end()){
        it->increaseFrame((it+1)->getFrameLength());
        this->noteSequence.erase(it+1);
        it = adjacent_find(it, this->noteSequence.end(), pred);
    }
    if(this->noteSequence.end()->getPitch() == NotePitch::rest){
        this->noteSequence.erase(this->noteSequence.end());
    }
}

string Melody::toString(){
    string quote = "";
    if(key.note < 12){
        quote += key.toString() + "\n";
    }
    for(size_t i = 0; i < this->noteSequence.size(); i++){
        quote += this->noteSequence[i].toString() + "\n";
    }
    return quote;
}

double correlationCoefficient(double* X, vector<int>& Y, int n) {
    double sum_X = 0, sum_Y = 0, sum_XY = 0;
    double squareSum_X = 0, squareSum_Y = 0;

    for (size_t i = 0; i < n; i++)
    {
        sum_X = sum_X + X[i];
        sum_Y = sum_Y + Y[i];
        sum_XY = sum_XY + X[i] * Y[i];
        squareSum_X = squareSum_X + X[i] * X[i];
        squareSum_Y = squareSum_Y + Y[i] * Y[i];
    }
    double corr = (double)(n * sum_XY - sum_X * sum_Y)
                  / sqrt((n * squareSum_X - sum_X * sum_X)
                      * (n * squareSum_Y - sum_Y * sum_Y));
    return corr;
}

void Melody::calcKey(){
    double major_profile[] = {6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88};
    double minor_profile[] = {6.33,	2.68, 3.52,	5.38, 2.60,	3.53, 2.54,	4.75, 3.98,	2.69, 3.34,	3.17};
    vector<int> noteProfile(12);
    vector<double> major_r(12);
    vector<double> minor_r(12);
    calcPitchProfile(noteProfile);
    for(size_t i = 0; i < major_r.size(); ++i){
        major_r[i] = correlationCoefficient(major_profile, noteProfile,major_r.size());
        minor_r[i] = correlationCoefficient(minor_profile, noteProfile,major_r.size());
        rotate(noteProfile.begin(), noteProfile.begin() + 1, noteProfile.end());
    }
    auto max_major = max_element(major_r.begin(),major_r.end());
    auto max_major_idx = distance(major_r.begin(),max_major);

    auto max_minor = max_element(minor_r.begin(),minor_r.end());
    auto max_minor_idx = distance(minor_r.begin(),max_minor);
    if( *max_major >= *max_minor){
        this->key.setPitch(max_major_idx);
        this->key.setMajor(true);
    }else{
        this->key.setPitch(max_minor_idx);
        this->key.setMajor(false);
    }
    cout << key.note << endl;
}

void Melody::calcPitchProfile(vector<int> &noteProfile){
    for(size_t i = 0; i < this->noteSequence.size(); i++){
        int capt_pitch = this->noteSequence[i].getPitch();
        if(capt_pitch != NotePitch::rest){
            noteProfile[capt_pitch] += this->noteSequence[i].getFrameLength();
        }
    }
}

