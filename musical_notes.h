#ifndef MUSICAL_NOTES_H_INCLUDED
#define MUSICAL_NOTES_H_INCLUDED

#include <iostream>
#include <vector>
#include <string>
#define Fs 44100
using namespace std;

enum NotePitch{
    C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B, rest = -1
};


class NoteUnit{
    private:
        int pitch;
        int octave;
    public:
        NoteUnit(int _pitch, int _octave);
        NoteUnit(int offset, NoteUnit ref_note);
        int getPitch();
        int getOctave();
        void shiftNote(int offset);
        void transpose(int octave_shift);
        void makeRestNote();
        string toString();
        bool equal(NoteUnit b);
        int distance(NoteUnit b);
};

class Note : public NoteUnit{
    private:
        int frameLength;
        double timeLength;
        int duration;
    public:
        Note(int pitch, int octave);
        void increaseFrame();
        void increaseFrame(int frame);
        string toString();
        int getFrameLength();

};

class Key{
    public:
        int note;
        bool isMajor; //Major = true
        void setPitch(int _note){
            this->note = _note;
        }
        void setMajor(bool _isMajor){
            this->isMajor = _isMajor;
        }
        string toString(){
            string note_str = "";
            switch(this->note){
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
            }
            return "Key : " + note_str + (isMajor ? "" : "m");
        }
};

void transformNotes(vector<int> &note_offset, vector<NoteUnit> &detected_notes, vector<bool> &VADresult, NoteUnit ref_note);

#endif // MUSICAL_NOTES_H_INCLUDED
