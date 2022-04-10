#import "MKMIDIEvent.h"

@implementation MKMIDIEvent

+(id)eventWithDeltaTimeInQuarters:(float)numQuarters
		    framesPerBeat:(unsigned int)frames
		       statusByte:(mkMIDIStatusByte_t)status
		    argumentArray:(unsigned char[])args
		      numArgumets:(unsigned int)size
{
  MKMIDIEvent* event = [[MKMIDIEvent alloc] init];
  [event setDeltaTimeWithNumQuarters: numQuarters framesPerBeat: frames];
  [event setStatusByte: status];
  int i;
  for(i=0; i<size; i++) [event addArgument: args[i]];

  return event;
}

-(id)init
{
  self = [super init];
  if(self != nil)
    {
      deltaTimeArray = [[MKArray alloc] init];
      argumentArray = [[MKArray alloc] init];
    }
  return self;
}

-(void)setDeltaTimeWithVariableLengthArray:(unsigned char[])array ofSize:(unsigned char)size
{
  if(size>4) return;
  [deltaTimeArray clear];
  unsigned char i;
  for(i=0; i<size; i++) [deltaTimeArray addByte: array[i]];
}

-(void)setDeltaTimeWithNumFrames:(unsigned int)numFrames
{
  if(numFrames > 268435456) numFrames = 0; //{fprintf(stderr, "delta time cannot excede 268435456\n"); return 0;}
  
  int size=1;
  unsigned char buffer[4];
  buffer[3] = (numFrames & 0x7F);
  numFrames>>=7;

  while(numFrames)
    {
      buffer[3-size] = ((numFrames & 0x7F) | 0x80);
      numFrames>>=7;
      size++;
    }
  //buffer += (4-size);
  unsigned char *ptr = &buffer[4-size];
  [self setDeltaTimeWithVariableLengthArray: ptr ofSize: size];
}

-(void)setDeltaTimeWithNumQuarters:(float)numQuarters framesPerBeat:(unsigned short int)frames
{
  [self setDeltaTimeWithNumFrames: (unsigned int)(numQuarters * (float)frames)]; 
}

-(MKArray*)deltaTimeAsVariableLengthArray
{
  return deltaTimeArray;
}

-(unsigned int)deltaTimeAsNumFrames
{
  unsigned int numFrames = 0;
  if([deltaTimeArray resetIterator])
    {
      do{
	numFrames <<=7;
	numFrames |= ([deltaTimeArray currentByte] & 0x7F);
      }while([deltaTimeArray advanceIterator: 1]);
    }
  return numFrames;
}

-(float)deltaTimeAsNumQuartersWithFramesPerBeat:(unsigned short int)frames;
{
  return (((float)[self deltaTimeAsNumFrames]) / (float)frames);
}

-(mkMIDIStatusByte_t)statusByte{return statusByte;}
-(mkMIDIStatusByte_t)statusNibble{return (statusByte & 0xF0);}
-(mkMIDIChannel_t)channel{return(statusByte & 0x0F);}

-(void)setStatusByte:(mkMIDIStatusByte_t)value{statusByte = value;}
-(void)setStatusNibble:(mkMIDIStatusByte_t)value
{
  statusByte &= 0x0F;
  value &= 0xF0;
  statusByte |= value;
}

-(void)setChannel:(mkMIDIChannel_t)value
{
  statusByte &= 0xF0;
  value &= 0x0F;
  statusByte |= value;
}

-(MKArray*)argumentArray
{
  return argumentArray;
}

-(unsigned char)argumentAtIndex:(mkMIDIEventArgumentIndex_t)index
{
  return [argumentArray byteAtIndex: index];
}

-(void)setArgument:(unsigned char)argument atIndex:(mkMIDIEventArgumentIndex_t)index
{
  unsigned int count = [argumentArray count];
  if(index >= count)
    {
      for(count=index-count; count>=0; count--)
	{
	  [self addArgument: 0];
	}
    }
  [argumentArray setByte: argument atIndex: index];
}

-(void)addArgument:(unsigned char)argument
{
  [argumentArray addByte: argument];
}

-(unsigned int)numBytes
{
  return [deltaTimeArray count] + 1 + [argumentArray count];
}

-(unsigned int)putEventIntoBuffer:(unsigned char[])buffer withOffset:(unsigned long int)offset
{
  buffer += offset;
  int i;
  for(i=0; i<[deltaTimeArray count]; i++)
    *(buffer++) = [deltaTimeArray byteAtIndex: i];
  *(buffer++) = statusByte;  
  for(i=0; i<[argumentArray count]; i++)
    *(buffer++) = [argumentArray byteAtIndex: i];
  
  return [self numBytes];
}

-(id)free
{
  [deltaTimeArray free];
  [argumentArray free];
  return [super free];
}

@end

