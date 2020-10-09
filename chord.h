#ifndef CHORD_H_INCLUDED
#define CHORD_H_INCLUDED
#include "Chord-Detector-and-Chromagram\src\ChordDetector.h"
#include "musical_notes.h"
#include <string>
#include <iostream>
#include <vector>

class ChordUnit : public NoteUnit{
    private:
        int chordType;
        int intervals;
    public:
        ChordUnit(int pitch, int octave, int chord_type, int intervals);
        string toString();
};

class Chord : public Note{
    private:
        int chordType;
        int intervals;
    public:

};

#endif // CHORD_H_INCLUDED
