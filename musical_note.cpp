#include "musical_notes.h"

//Method Definition of NoteUnit Class
NoteUnit::NoteUnit(int _pitch, int _octave){
            this->pitch = _pitch;
            this->octave = _octave;
}
NoteUnit::NoteUnit(int offset, NoteUnit ref_note){
            this->pitch = ref_note.getPitch();
            this->octave = ref_note.getOctave();
            shiftNote(offset);
}
int NoteUnit::getPitch(){
    return pitch;
}
int NoteUnit::getOctave(){
    return octave;
}
void NoteUnit::shiftNote(int offset){
    int octave_shift = offset / 12;
    int pitch_shift = offset % 12;
    this->octave += octave_shift;
    int temp = (this->pitch + pitch_shift) % 12;
    this->pitch = temp;
}
void NoteUnit::transpose(int octave_shift){
    octave += octave_shift;
}
void NoteUnit::makeRestNote(){
    this->pitch = NotePitch::rest;
    this->octave = -1;
}
bool NoteUnit::equal(NoteUnit b){
    return (this->getPitch() == b.getPitch()) && (this->getOctave() == b.getOctave());
}

int NoteUnit::distance(NoteUnit b){
    int octave_diff = b.getOctave() - this->getOctave();
    octave_diff*=12;
    int pitch_diff = b.getPitch() - this->getPitch();
    return octave_diff+pitch_diff;
}
string NoteUnit::toString(){
    string note_str;
    string octave_str;
    switch(this->pitch){
        case C : note_str = "C"; break;
        case Db: note_str = "C#"; break;
        case D : note_str = "D"; break;
        case Eb: note_str = "D#"; break;
        case E : note_str = "E"; break;
        case F : note_str = "F"; break;
        case Gb: note_str = "F#"; break;
        case G : note_str = "G"; break;
        case Ab: note_str = "G#"; break;
        case A : note_str = "A"; break;
        case Bb: note_str = "A#"; break;
        case B : note_str = "B"; break;
        case rest : note_str = "-"; break;
    }
    if(this->octave == -1 || this->pitch == rest)
        octave_str = "";
    else
        octave_str = to_string(this->octave);
    return note_str + octave_str;
}

//Method Definition of Note Class
Note::Note(int _pitch, int _octave) : NoteUnit(_pitch, _octave){
    this->frameLength = 1;
    this->duration = -1;
    this->timeLength = -1.00;
}

void Note::increaseFrame(){
    ++this->frameLength;
}

void Note::increaseFrame(int frame){
    this->frameLength += frame;
}

int Note::getFrameLength(){
    return this->frameLength;
}

string Note::toString(){
    string note = NoteUnit::toString();
    string frameLength = "\tframeLen : "  + to_string(this->frameLength);
    string timeLength = "\ttimeLen : "  + to_string(this->timeLength);
    string duration = "\t\tduration : " + to_string(this->duration);
    return note + frameLength + duration + timeLength;
}


void transformNotes(vector<int> &note_offset, vector<NoteUnit> &detected_notes, vector<bool> &vad_result, NoteUnit ref_note){
    if(note_offset.size() != vad_result.size()){
        cout << "Could not transforms" << endl;
        return;
    }
    for(size_t i = 0; i < note_offset.size(); i++){
        NoteUnit temp(note_offset[i], ref_note);
        if(vad_result[i] == 0)
            temp.makeRestNote();
        detected_notes.push_back(temp);
    }

}
