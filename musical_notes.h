#ifndef MUSICAL_NOTES_H_INCLUDED
#define MUSICAL_NOTES_H_INCLUDED

#include <iostream>
#include <vector>
#define Fs 44100

enum NotePitch{
    C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B
};

class NoteUnit{
    private:
        NotePitch pitch;
        int octave;
    public:
        NoteUnit(NotePitch _pitch, int _octave){
            this.pitch = _pitch;
            this.octave = _octave;
        }
        NoteUnit(int offset, NoteUnit ref_note){
            this.pitch = ref_note.getPitch();
            this.octave = ref_note.getOctave();
            shiftNote(offset);
        }
        NotePitch getPitch() return pitch;
        int getOctave() return octave;
        void shiftNote(int offset){
            int octave_shift = offset / 12;
            int pitch_shift = offset % 12;
            octave += octave_shift;
            pitch = (pitch + pitch_shift) % 12;
        }
        void transpose(int octave_shift){
            octave += octave_shift;
        }
        void makeRestNote(){
            this.pitch = -1;
            this.octave = -1;
        }

};

class Note : public NoteUnit{
    private:
        int frameLength
        double timeLength
        int duration
    public:
        Note(NotePitch pitch, int octave);
        void increaseFrame();
};

class Chord : public Note{
    private:
        string chordType
    public:
        Chord(NotePitch, int octave, string chord_type);
};

class Key{
    private:
        NoteUnit note;
        bool isMajor;
    public:
        Key(NoteUnit key, bool setMinor)
};

class trackSequence{
    private:
        Key key;
        int tempo;
        int frameLength;
        int trackDuration;
    public:
        virtual Key calcKey() = 0;
        virtual int calcTempo() = 0;
        virtual int calcFrameLength() = 0;
        virtual int calcDurations() = 0;
        virtual void cleanTrack() = 0; //use if final note are no-note
        virtual void writeFile() = 0;
        virtual void updateSequence() = 0; //2.3
        virtual void removeUnit(int position) // 0, -1 ,2 ,3
};

class Melody : trackSequence{
    private:
        vector<Note> noteSequence;
    public:
        Key calcKey();
        int calcTempo();
        int calcFrameLength();
        int calcDurations()0;
        void cleanTrack(); //use if final note are no-note
        void writeFile();
        void updateSequence(); //2.3
        void removeUnit(int position); // 0, -1 , -2
        void registerNewNote(Note newNote);
};

class ChordProgression : trackSequence{
    private:
        vector<Chord> chordSequence;
    public:
        Key calcKey();
        int calcTempo();
        int calcFrameLength();
        int calcDurations()0;
        void cleanTrack(); //use if final note are no-note
        void writeFile();
        void updateSequence(); //2.3
        void removeUnit(int position); // 0, -1 , -2
        void registerNewChord(Note newNote);
};


#endif // MUSICAL_NOTES_H_INCLUDED
