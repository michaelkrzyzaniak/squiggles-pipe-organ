
/*-------------------------------------------------------------------------------
This file is the class reference for MKMIDIFile.

The code contained in this file is available free of charge, lisence, or 
restriction of any kind. Anyone may use it for anything, although it will 
probably crash your computer irreparably.  

Written by Michael Krzyzaniak 
krzyzani@uga.edu
-------------------------------------------------------------------------------*/

#import <stdio.h>// &#60stdio.h&#62
#import <string.h>//&#60string.h&#62
#import "MKMIDIStatusBytes.h"
#import "MKMIDIEventList.h"
#import "MKArray.h"

typedef enum 
  {
    MONOPHONIC_FORMAT_TYPE = 0,
    POLYPHONIC_FORMAT_TYPE = 1,
    TYPE_THREE_FORMAT_TYPE = 2,
  }mkMIDIFormatType_t;

typedef enum
  {
    HEADER_CHUNK_ID = 'MThd',
    TRACK_CHUNK_ID = 'MTrk',
  }mkMIDIChunkIdentifier_t;

typedef struct
{
  mkMIDIChunkIdentifier_t chunkID;
  uint32_t  chunkSize;
  uint16_t formatType;
  uint16_t numTracks;
  uint16_t timeDivision;
}mkMIDIHeaderChunk;

typedef struct
{
  mkMIDIChunkIdentifier_t chunkID;
  uint32_t chunkSize;
}mkMIDITrackChunk;

#define HEADER_CHUNK_SIZE  6

@interface MKMIDIFile : Object
{
  MKArray* trackArray;
  MKArray* previousRest;
  unsigned short int framesPerBeat;
}

//creating and initializing
+(id)midiFileFromFile:(char*)filename;
-(id)initWithNumTracks:(unsigned int)numTracks;
-(id)init;

//adding events
-(void)addTracks:(int)numTracks;
-(void)addNoteOn:(mkMIDIPitch_t)num 
	withChan:(mkMIDIChannel_t)chan 
	     vel:(mkMIDIVelocity_t)vel
afterDurationInQuarters:(float)numQuarters
	 toTrack:(unsigned int)track;
-(void)addNoteOff:(mkMIDIPitch_t)num 
	 withChan:(mkMIDIChannel_t)chan 
 afterDurationInQuarters:(float)numQuarters
	  toTrack:(unsigned int)track; 
-(void)addNote:(mkMIDIPitch_t)num
      withChan:(mkMIDIChannel_t)chan
	   vel:(mkMIDIVelocity_t)vel
durationInQuarters:(float)numQuarters
       toTrack:(unsigned int)track;
-(void)addRestWithDurationInQuarters:(float)numQuarters
			     toTrack:(unsigned int)track;
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

//iterating over contents of track
-(BOOL)resetIteratorOnTrack:(unsigned int)track;
-(BOOL)advanceIteratorOnTrack:(unsigned int)track;
-(MKMIDIEvent*)currentEventOnTrack:(unsigned int)track;

//getting info
-(unsigned int)numTracks;
-(float)durationInQuarters;
-(unsigned char)shortestNoteOnTrack:(unsigned int)track;
-(BOOL)containsNotesOnTrack:(unsigned int)track; //some tracks will contain only meta-data
-(unsigned short)framesPerBeat;

//misc
-(void)quantizeToDuration:(unsigned char)duration; //16 for 16ths, 4 for quarters...
-(BOOL)saveWithFilename:(char*)name;
@end
