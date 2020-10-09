#ifndef TRACK_SEQUENCE_H_INCLUDED
#define TRACK_SEQUENCE_H_INCLUDED
#include "musical_notes.h"
#include "chord.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>


class Melody{
    private:
        size_t numOfFrame;
        Key key;
        int tempo;
        int frameLength;
        int trackDuration;
        int frameStart;
        int frameEnd;

    public:
        Melody(vector<NoteUnit> &raw_note);
        vector<Note> noteSequence;
        string toString();
        void calcKey();
        int calcTempo();
        int calcFrameLength();
        int calcDurations();
        void calcPitchProfile(vector<int> &noteProfile);
        void cleanTrack(); //use if final note are no-note
        void writeFile();
        void updateSequence(); //2.3
        void removeUnit(int position); // 0, -1 , -2
        void registerNewNote(Note newNote);
};

class ChordProgression{
    private:
        vector<Chord> chordSequence;
    public:
        Key calcKey();
        int calcTempo();
        int calcFrameLength();
        int calcDurations();
        void cleanTrack(); //use if final note are no-note
        void writeFile();
        void updateSequence(); //2.3
        void removeUnit(int position); // 0, -1 , -2
        void registerNewChord(Note newNote);
};

#endif // TRACK_SEQUENCE_H_INCLUDED
