#import "MKArray.h"
#import "MKMIDIStatusBytes.h"

typedef enum
{
  NOTE_NUMBER_INDEX = 0,
  VELOCITY_INDEX = 1,
  
  
}mkMIDIEventArgumentIndex_t;

@interface MKMIDIEvent : Object
{
  MKArray* deltaTimeArray;
  unsigned char statusByte;
  MKArray* argumentArray;
} 

+(id)eventWithDeltaTimeInQuarters:(float)numQuarters
		    framesPerBeat:(unsigned int)frames
		       statusByte:(mkMIDIStatusByte_t)status
		    argumentArray:(unsigned char[])args
		      numArgumets:(unsigned int)size;

-(void)setDeltaTimeWithVariableLengthArray:(unsigned char[])array ofSize:(unsigned char)size;
-(void)setDeltaTimeWithNumFrames:(unsigned int)numFrames;
-(void)setDeltaTimeWithNumQuarters:(float)numBeats framesPerBeat:(unsigned short int)frames;
-(MKArray*)deltaTimeAsVariableLengthArray;
-(unsigned int)deltaTimeAsNumFrames;
-(float)deltaTimeAsNumQuartersWithFramesPerBeat:(unsigned short int)frames;

-(mkMIDIStatusByte_t)statusByte;
-(mkMIDIStatusByte_t)statusNibble;
-(mkMIDIChannel_t)channel;
-(void)setStatusByte:(mkMIDIStatusByte_t)value;
-(void)setStatusNibble:(mkMIDIStatusByte_t)value;
-(void)setChannel:(mkMIDIChannel_t)value;

-(MKArray*)argumentArray;
-(unsigned char)argumentAtIndex:(mkMIDIEventArgumentIndex_t)index;
-(void)setArgument:(unsigned char)argument atIndex:(mkMIDIEventArgumentIndex_t)index; 
-(void)addArgument:(unsigned char)argument;

-(unsigned int)numBytes;
-(unsigned int)putEventIntoBuffer:(unsigned char[])buffer withOffset:(unsigned long int)offset;  //returns numBytes writen;

@end
