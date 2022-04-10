
/*-------------------------------------------------------------------------------
This file demonstrates how to use MKMIDIFile to create a .mid file 
algorithmically. The program creates a MIDI file containing a few measures of 
Beethoven's Opus 49 and saves it wherever you invoked the program (probably in 
your home directory). MKMIDIFile always uses track numbers starting at index 1.

The code contained in this file is available free of charge, lisence, or 
restriction of any kind. Anyone may use it for anything, although it will 
probably crash your computer irreparably.  

Written by Michael Krzyzaniak 
krzyzani@uga.edu
-------------------------------------------------------------------------------*/

#import "MKMIDIFile.h"

int main()
{  
  unsigned char firstPitchArray[] = {MIDI_PITCH_G_4, MIDI_PITCH_G_4, 
				     MIDI_PITCH_Fs_4, MIDI_PITCH_G_4, 
				     MIDI_PITCH_A_4, MIDI_PITCH_A_4, 
                                     MIDI_PITCH_B_4, MIDI_PITCH_B_4, 
				     MIDI_PITCH_D_5, MIDI_PITCH_C_5, 
				     MIDI_PITCH_B_4};
  unsigned char secondPitchArray[] = {MIDI_PITCH_B_3, MIDI_PITCH_A_3, 
				      MIDI_PITCH_G_3, MIDI_PITCH_Fs_3, 
				      MIDI_PITCH_D_3, MIDI_PITCH_G_3, 
                                      MIDI_PITCH_F_3, MIDI_PITCH_E_3, 
				      MIDI_PITCH_Fs_3, MIDI_PITCH_G_3};
  float firstRhythmArray[] = {1, 1, 1, 1, 1.5, 0.5, 1.5, 0.5, 1, 1, 1};
  unsigned int i;
  
  MKMIDIFile* midiFile = [[MKMIDIFile alloc] initWithNumTracks: 2];
 
  [midiFile addTempoChange: 100.0 toTrack: 1];
  [midiFile addMeterChangeWithNumerator: 2 
	    denominator: 2 
	    thirtySecondsPerQuarter: 8 
	    toTrack: 1];
  [midiFile addKeyChangeWithNumAccidentals: 1 sharps: YES major: YES toTrack: 1];
  [midiFile addCopyright: "Copyright L. Beethoven, 1795" toTrack: 1];

  for(i=0; i<sizeof(firstPitchArray); i++) //>&#60sizeof(firstPitchArray); i++)
    {
      [midiFile addNote: firstPitchArray[i]
		withChan: CHANNEL_1
		vel: VELOCITY_MF
		durationInQuarters: firstRhythmArray[i]
		toTrack: 1];
    }

  [midiFile addRestWithDurationInQuarters: 1 toTrack: 2];

  for(i=0; i<sizeof(secondPitchArray); i++)//>&#60sizeof(secondPitchArray); i++)
    {
      [midiFile addNote: secondPitchArray[i]
		withChan: CHANNEL_1
		vel: VELOCITY_MP
		durationInQuarters: 1
		toTrack: 2];
    }

  [midiFile saveWithFilename: "Opus49.mid"];
  [midiFile free];
  return 0;	
}
