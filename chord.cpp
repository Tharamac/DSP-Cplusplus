#include "chord.h"
#include "musical_notes.h"
ChordUnit::ChordUnit(int pitch, int octave, int chord_type, int intervals) : NoteUnit(pitch, octave){
    this->chordType = chord_type;
    this->intervals = intervals;
}

string ChordUnit::toString(){
    string note_str;
    string octave_str;
    string chordType;
    string interval;
    int _pitch = this->getPitch();
    int _octave = this->getOctave();

    switch(_pitch){
        case NotePitch::C : note_str = "C"; break;
        case NotePitch::Db: note_str = "C#"; break;
        case NotePitch::D : note_str = "D"; break;
        case NotePitch::Eb: note_str = "D#"; break;
        case NotePitch::E : note_str = "E"; break;
        case NotePitch::F : note_str = "F"; break;
        case NotePitch::Gb: note_str = "F#"; break;
        case NotePitch::G : note_str = "G"; break;
        case NotePitch::Ab: note_str = "G#"; break;
        case NotePitch::A : note_str = "A"; break;
        case NotePitch::Bb: note_str = "A#"; break;
        case NotePitch::B : note_str = "B"; break;
        case NotePitch::rest : note_str = "-"; break;
    };
    if(_octave == -1 || _pitch == rest)
        octave_str = "";
    else
        octave_str = to_string(_octave);
    switch(this->chordType){
        case ChordDetector::ChordQuality::Minor : chordType = "Minor";break;
        case ChordDetector::ChordQuality::Major : chordType = "Major";break;
        case ChordDetector::ChordQuality::Suspended : chordType = "Sus";break;
        case ChordDetector::ChordQuality::Dominant : chordType = "Dom";break;
        case ChordDetector::ChordQuality::Dimished5th : chordType = "Dim5th";break;
        case ChordDetector::ChordQuality::Augmented5th : chordType = "Aug5th";break;
    };
    switch(this->intervals){
        case 0: interval = "Triad"; break;
        case 2: interval = "2"; break;
        case 4: interval = "4"; break;
        case 7: interval = "7th";break;
    };
    return note_str + octave_str + " " + chordType + " " + interval;
}
