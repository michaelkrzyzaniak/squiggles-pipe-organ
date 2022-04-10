
#import "MKMIDIFile.h"

BOOL thisSystemIsLittleEndian()
{
  int test = 1;
  return *((char*)&test);
}

void byteSwap16(uint16_t* value)
{
  *value = ((*value << 8) | (*value >> 8));
}

void byteSwap32(uint32_t* value)
{
  char *array = (char*)value;
  unsigned char storage;
  char i = 0;
  char j = sizeof(value);
  
  for(i=0; i<(j/2); i++)
    {
      storage = *(array + i);
      *(array + i) = *(array + (j-i-1));
      *(array + (j-i-1)) = storage;
    }
  *value = *((int*)array);
}

void convertHeaderByteOrder(mkMIDIHeaderChunk* header)
{
  if(thisSystemIsLittleEndian())
    {
      byteSwap32(&(header->chunkID));
      byteSwap32(&(header->chunkSize));
      byteSwap16(&(header->formatType));
      byteSwap16(&(header->numTracks));
      byteSwap16(&(header->timeDivision));
    }
}

void convertTrackByteOrder(mkMIDITrackChunk* track)
{
  if(thisSystemIsLittleEndian())
    {
      byteSwap32(&(track->chunkID));
      byteSwap32(&(track->chunkSize));
    }
}

@interface MKMIDIFile (Private)

-(void)addString:(char*)string toTrack:(unsigned int)track withType:(mkMIDIMetaEvent_t)type;
-(float)getPreviousRestOnTrack:(unsigned int)track;
-(void)removeEndOfTrackFromAllTracks;
-(void)endAllTracks;
-(void)setFramesPerBeat:(unsigned short int)value;
-(void)parseTrackData: (unsigned char[])trackData ofSize:(unsigned int)trackSize;

@end

@implementation MKMIDIFile

-(float)getPreviousRestOnTrack:(unsigned int)track
{
  float value = [previousRest floatAtIndex: track-1];
  [previousRest setFloat: 0.0 atIndex: track-1];
  return value;
}

+(id)midiFileFromFile:(char*)filename
{
  FILE* file = fopen(filename, "r");
  if(file == nil) { fprintf(stderr,"could not open %s\n", filename); return nil; }
  mkMIDIHeaderChunk header;
  fread(&header, sizeof(header)-2, 1, file);
  convertHeaderByteOrder(&header);
  if(header.chunkID != HEADER_CHUNK_ID) { fprintf(stderr,"%s is not a valid MIDI file\n", filename); return nil; }
  MKMIDIFile* midiFile = [[MKMIDIFile alloc] init];
  [midiFile setFramesPerBeat: header.timeDivision];
  unsigned int i;
  for(i=0; i<header.numTracks; i++)
    {
      mkMIDITrackChunk track; 
      fread(&track, sizeof(track), 1, file);
      convertTrackByteOrder(&track);
      unsigned char trackData[track.chunkSize];
      fread(&trackData, sizeof(trackData), 1, file);
      [midiFile parseTrackData: trackData ofSize: sizeof(trackData)];
    }

  [midiFile removeEndOfTrackFromAllTracks];
 return midiFile; 
}

-(void)parseTrackData: (unsigned char[])trackData ofSize:(unsigned int)trackSize
{
  [self addTracks: 1];
  unsigned int numBytesRead = 0;
  unsigned char deltaTime[4];
  unsigned char messageSize, previousStatus;
  
  while(numBytesRead < trackSize)
    {
      MKMIDIEvent* event = [[MKMIDIEvent alloc] init];
      messageSize = 0;
      
      //get delta time
      while(trackData[0] > 127)
      {
	deltaTime[messageSize] = *(trackData++);
	messageSize++;
	numBytesRead++;
      }
      deltaTime[messageSize] = *(trackData++);
      messageSize++;
      numBytesRead++;
      [event setDeltaTimeWithVariableLengthArray: deltaTime ofSize: messageSize];
    
      //get status byte
      if(trackData[0]>=0x80)  {  previousStatus = *(trackData++); numBytesRead++;  }
      [event setStatusByte: previousStatus];

      //get arguments;
      if((previousStatus & 0xF0) == NOTE_ON)
	messageSize = 2;
      else if((previousStatus & 0xF0) == NOTE_OFF)
	messageSize = 2;
      else if((previousStatus & 0xF0) == POLYPHONIC_AFTERTOUCH)
	messageSize = 2;
      else if(previousStatus == PITCH_WHEEL)
	messageSize = 2;
      else if((previousStatus & 0xF0) == MODE_CHANGE)
	messageSize = 2;
      else if((previousStatus & 0xF0) == PROGRAM_CHANGE)
	messageSize = 1;
      else if((previousStatus & 0xF0) == CHANNEL_AFTERTOUCH)
	messageSize = 1;
      else if(previousStatus == META_EVENT)
	messageSize = trackData[1]+2;  
      else if(previousStatus == SYSTEM_EXCLUSIVE)
	{
	  messageSize = 1;
	  while(trackData[messageSize-1] != END_OF_EXCLUSIVE)  
	    {messageSize++;}
	  continue;
	}
      else continue;

      for(messageSize; messageSize>0; messageSize--)
	{
	  [event addArgument: *(trackData++)];
	  numBytesRead++;
	}

      //fix note off messages      
      if((([event statusByte] &0xF0) == NOTE_ON) && ([[event argumentArray] byteAtIndex:1] == 0x00))
	{
	  [event setStatusByte: [event statusByte] - 0x10]; 
	}
            
      /*       //debug messages
      printf("event added with NumQuarters: %f statusByte: %02x, numArgs: %u, firstArg: %02x\n", 
	     [event deltaTimeAsNumQuartersWithFramesPerBeat: framesPerBeat],
	     [event statusByte],
	     [[event argumentArray] count],
	     [event argumentAtIndex: 0]);
      */  
      [[trackArray objectAtIndex: [trackArray count]-1] addEvent: event];
    }
}

-(id)init
{
  return [self initWithNumTracks: 0];
}

-(id)initWithNumTracks:(unsigned int)numTracks
{
  if(self=[super init])
    {
      trackArray = [[MKArray alloc]init];
      previousRest = [[MKArray alloc]init];
      framesPerBeat = 240;
      [self addTracks: numTracks];
    }
  return self;
}

-(void)addTracks:(int)numTracks
{
  for(numTracks; numTracks>0; numTracks--)
    {
      [trackArray addObject: [[MKMIDIEventList alloc]init]];
      [previousRest addFloat: 0.0];
    }
}

-(void)endAllTracks
{
  int i;
  unsigned char event[3];
  event[0] = META_EVENT;
  event[1] = END_OF_TRACK;
  event[2] = 0x00; //size of data to follow
  for(i=0; i<[trackArray count]; i++)
    {
      [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: 4 toTrack: i+1];
    }
}

-(void)removeEndOfTrackFromAllTracks
{
  if([trackArray resetIterator])
    {
      do{
	MKMIDIEvent* event = [[trackArray currentObject] lastObject];
	if(([event statusByte] == META_EVENT) && ([[event argumentArray] byteAtIndex: 0] == END_OF_TRACK))
	  [[trackArray currentObject] removeLastEvent];
      }while([trackArray advanceIterator: 1]);
    }
}

-(BOOL)saveWithFilename:(char*)name
{
  unsigned int i;
  [self endAllTracks];
  FILE* midiFile = fopen(name, "w+");
  if(midiFile == NULL)  {fprintf(stderr, "could not save MIDI file with name: %s\n", name); return NO;}
  mkMIDIHeaderChunk midiHeader = {HEADER_CHUNK_ID, 
				  HEADER_CHUNK_SIZE, 
				  POLYPHONIC_FORMAT_TYPE, 
				  [trackArray count], 
				  framesPerBeat};
  convertHeaderByteOrder(&midiHeader);
  fwrite(&midiHeader, sizeof(midiHeader)-2, 1, midiFile);
  for(i=0; i<[trackArray count]; i++)
    { 
      unsigned long int numBytes;
      unsigned char* eventData = [[trackArray objectAtIndex: i] createContentsArrayOfSize: &numBytes];
      mkMIDITrackChunk midiTrack = {TRACK_CHUNK_ID, numBytes};
      convertTrackByteOrder(&midiTrack);
      fwrite(&midiTrack, sizeof(midiTrack), 1, midiFile);
      fwrite(eventData, 1, numBytes, midiFile);
      free(eventData);
    }
  [self removeEndOfTrackFromAllTracks];
  fclose(midiFile);
  return YES;
}

-(void)addNoteOn:(mkMIDIPitch_t)num 
	withChan:(mkMIDIChannel_t)chan 
	     vel:(mkMIDIVelocity_t)vel
afterDurationInQuarters:(float)numQuarters
	 toTrack:(unsigned int)track
{
  unsigned char event[3];
  event[0] = NOTE_ON | chan;
  event[1] = num;
  event[2] = vel;
  [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: numQuarters toTrack: track];
}

-(void)addNoteOff:(mkMIDIPitch_t)num 
	 withChan:(mkMIDIChannel_t)chan 
 afterDurationInQuarters:(float)numQuarters
	  toTrack:(unsigned int)track
{
  unsigned char event[3];
  event[0] = NOTE_OFF | chan;
  event[1] = num;
  event[2] = VELOCITY_OFF;
  [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: numQuarters toTrack: track];
} 

-(void)addNote:(mkMIDIPitch_t)num
      withChan:(mkMIDIChannel_t)chan
	   vel:(mkMIDIVelocity_t)vel
durationInQuarters:(float)numQuarters
       toTrack:(unsigned int)track
{
  [self addNoteOn: num
	withChan: chan
	vel: vel
	afterDurationInQuarters: 0
	toTrack: track];

  [self addNoteOff: num
	withChan: chan
	afterDurationInQuarters: numQuarters
	toTrack: track];
}

-(void)addRestWithDurationInQuarters:(float)numQuarters
			     toTrack:(unsigned int)track
{
  float newValue = [previousRest floatAtIndex: track-1];
  newValue += numQuarters;
  [previousRest setFloat: newValue atIndex: track-1];
}

-(void)addString:(char*)string toTrack:(unsigned int)track withType:(mkMIDIMetaEvent_t)type
{
  MKMIDIEvent* event = [[MKMIDIEvent alloc] init];
  unsigned int i, size = strlen(string);
  [event setDeltaTimeWithNumQuarters: [self getPreviousRestOnTrack: track] framesPerBeat: framesPerBeat];
  [event setStatusByte: META_EVENT];
  [event addArgument: type];
  [event addArgument: size];
  for(i=0; i<size; i++)  [event addArgument: *(string+i)];

  [[trackArray objectAtIndex: track-1] addEvent: event];
}

-(void)addCopyright:(char*)string toTrack:(unsigned int)track
{
  [self addString: string toTrack: track withType: COPYRIGHT];
}

-(void)addText:(char*)string toTrack:(unsigned int)track
{
  [self addString: string toTrack: track withType: INSERT_LYRIC];
}

-(void)addLyric:(char*)string toTrack:(unsigned int)track
{
  [self addString: string toTrack: track withType: INSERT_TEXT];
}

-(void)addTrackName:(char*)string toTrack:(unsigned int)track
{
  [self addString: string toTrack: track withType: TRACK_NAME];
}
-(void)addInstrumentName:(char*)string toTrack:(unsigned int)track
{
  [self addString: string toTrack: track withType: INSTRUMENT_NAME];
}

-(void)addTempoChange:(float)bpm toTrack:(unsigned int)track
{
  unsigned char event[6];
  unsigned int microsecsPerQuarter = 60000000.0 / bpm;
  if(thisSystemIsLittleEndian()) byteSwap32(&microsecsPerQuarter);
  event[0] = META_EVENT;
  event[1] = TEMPO_CHANGE;
  event[2] = 0x03; //numBytesToFollow;
  event[3] = (microsecsPerQuarter >> 8) & 0xFF;
  event[4] = (microsecsPerQuarter >> 16) & 0xFF;
  event[5] = (microsecsPerQuarter >> 24) & 0xFF;

  [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: 0.0 toTrack: track];
}  

-(void)addMeterChangeWithNumerator:(unsigned char)numerator 
		    denominator:(unsigned char)denominator 
	thirtySecondsPerQuarter:(unsigned char) numThirtySeconds
			   toTrack:(unsigned int)track
{
  unsigned char event[7];
  unsigned char logDenominator = 0;
  while(denominator > 1) {denominator = (float)denominator/2.0; logDenominator++;}
  
  event[0] = META_EVENT;
  event[1] = TIME_SIGNATURE;
  event[2] = 4; //numBytes to follow
  event[3] = numerator;
  event[4] = logDenominator;
  event[5] = 24;
  event[6] = numThirtySeconds;

  [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: 0.0 toTrack: track];
}

-(void)addByteArray:(unsigned char[])array 
	     ofSize:(unsigned int)size 
      afterNumQuarters:(float)numQuarters
	    toTrack:(unsigned int)track 
{
  int i;
  numQuarters += [self getPreviousRestOnTrack: track];
  MKMIDIEvent* event = [MKMIDIEvent eventWithDeltaTimeInQuarters: numQuarters
				    framesPerBeat: framesPerBeat
				    statusByte: array[0]
				    argumentArray: array+1
				    numArgumets: size-1];
  
  [[trackArray objectAtIndex: track-1] addEvent: event];
}

-(void)addKeyChangeWithNumAccidentals:(char)numAccidentals sharps:(BOOL)sharps major:(BOOL)major toTrack:(unsigned int)track;
{
  if(!sharps) numAccidentals *= -1;
  unsigned char event[5];  
  event[0] = META_EVENT;
  event[1] = KEY_SIGNATURE;
  event[2] = 2; //numBytes to follow
  event[3] = (unsigned char)numAccidentals;
  event[4] = (unsigned char)major;
 
  [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: 0.0 toTrack: track];
}

-(void)quantizeToDuration:(unsigned char)duration
{
  if([trackArray resetIterator])
    {
      do{
	[[trackArray currentObject]quantizeToDuration: duration withFramesPerBeat: framesPerBeat];
      }while([trackArray advanceIterator: 1]);
    }
}

-(float)durationInQuarters
{
  float maxDuration = 0, currentDuration;
  if([trackArray resetIterator])
    {
      do{
	currentDuration = [[trackArray currentObject] durationInQuartersWithFramesPerBeat: framesPerBeat];
	if(currentDuration > maxDuration) maxDuration = currentDuration;
      }while([trackArray advanceIterator: 1]);
    }
  return maxDuration;
}

-(BOOL)resetIteratorOnTrack:(unsigned int)track
{
  return [[trackArray objectAtIndex: track-1] resetIterator];
}

-(BOOL)advanceIteratorOnTrack:(unsigned int)track
{
  return [[trackArray objectAtIndex: track-1] advanceIterator: 1];
}

-(MKMIDIEvent*)currentEventOnTrack:(unsigned int)track
{
  return [[trackArray objectAtIndex: track-1] currentObject];
}
-(unsigned char)shortestNoteOnTrack:(unsigned int)track
{
  return [[trackArray objectAtIndex: track-1] shortestNoteWithFramesPerBeat: framesPerBeat];
}

-(unsigned int)numTracks
{
  return [trackArray count];
}

-(BOOL)containsNotesOnTrack:(unsigned int)track
{
  return [[trackArray objectAtIndex: track-1] containsNotes];
}

-(MKMIDIEventList*)eventListOnTrack:(unsigned int)track
{
  return [trackArray objectAtIndex: track-1];
}

-(unsigned short)framesPerBeat
{
  return framesPerBeat;
}

-(void)setFramesPerBeat:(unsigned short int)value
{
  framesPerBeat = value;
}

-(id)free
{
  [trackArray free];
  [previousRest free];
  return [super free];
}

@end
