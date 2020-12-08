#import <stdio.h>
#import <string.h>
#import "MIDI_Parser.h"
#import "MIDI_Track.h"
#import "List.h"




typedef struct opaque_midi_file_struct MIDI_File;



//creating and initializing
MIDI_File* midi_file_new_from_file(char* filename);
MIDI_File* midi_file_new(uint16_t frames_per_beat /*should be 1680 by default*/);

MIDI_Track* midi_file_add_empty_tracks(MIDI_File* self, int num_tracks); //returns last track

void midi_file_append_track(MIDI_File* self, MIDI_Track* track);
/*
void midi_file_sequential_append_note_on(MIDI_File* self, midi_pitch_t note, midi_channel_t chan, midi_velocity_t vel, double after_num_beats, unsigned track);
void midi_file_sequential_append_note_off(MIDI_File* self, midi_pitch_t note, midi_channel_t chan, double after_num_beats, unsigned track);
void midi_file_sequential_append_note(MIDI_File* self, midi_pitch_t note, midi_channel_t chan, double duration_in_beats, unsigned track);
void midi_file_sequential_append_rest(MIDI_File* self, double duration_in_beats, unsigned track);

void midi_file_append_copyright(MIDI_File* self, char* str, unsigned track);
void midi_file_append_text     (MIDI_File* self, char* str, unsigned track);
void midi_file_append_lyric    (MIDI_File* self, char* str, unsigned track);
void midi_file_append_track_name(MIDI_File* self, char* str, unsigned track);
void midi_file_append_instrument_name(MIDI_File* self, char* str, unsigned track);
void midi_file_append_tempo_change(MIDI_File* self, char* str, unsigned track);
*/
/*
-(void)addCopyright:(char*)string toTrack:(unsigned int)track;
-(void)addText:(char*)string toTrack:(unsigned int)track;
-(void)addLyric:(char*)string toTrack:(unsigned int)track;
-(void)addTrackName:(char*)string toTrack:(unsigned int)track;
-(void)addInstrumentName:(char*)string toTrack:(unsigned int)track;
-(void)addTempoChange:(float)bpm toTrack:(unsigned int)track; 
-(void)addMeterChangeWithNumerator:(unsigned char)numerator 
		       denominator:(unsigned char)denominator 
	   thirtySecondsPerQuarter:(unsigned char) numThirtySeconds
			   toTrack:(unsigned int)track;
-(void)addByteArray:(unsigned char[])array 
	     ofSize:(unsigned int)size 
      afterNumQuarters:(float)numQuarters
	    toTrack:(unsigned int)track; 
-(void)addKeyChangeWithNumAccidentals:(char)numAccidentals 
			       sharps:(BOOL)sharps 
				major:(BOOL)major 
			      toTrack:(unsigned int)track;
*/

//iterating over contents of track
/*
-(BOOL)resetIteratorOnTrack:(unsigned int)track;
-(BOOL)advanceIteratorOnTrack:(unsigned int)track;
-(MKMIDIEvent*)currentEventOnTrack:(unsigned int)track;
*/

//getting info

unsigned  midi_file_get_num_tracks(MIDI_File* self);
List*        midi_file_get_track_list(MIDI_File* self);
MIDI_Track*  midi_file_get_track_at_index(MIDI_File* self, uint16_t index);

float duration_in_quarters(MIDI_File* self);
int   midi_file_get_frames_per_beat(MIDI_File* self);
//set frames per beat requires resampling everything (add argument resample = yes or no);

//misc
//-(void)quantizeToDuration:(unsigned char)duration; //16 for 16ths, 4 for quarters...
BOOL midi_file_save_with_filename(MIDI_File* self, char* name);

MIDI_File* midi_file_destroy(MIDI_File* self);


