#import"MKMIDIEventList.h"

@implementation MKMIDIEventList

-(id)init
{
  self = [super init];
  if(self != nil)
    {

    }
  return self;
}

-(void)addEvent:(MKMIDIEvent*)event
{  
  [super addObject: event];
}

-(void)removeLastEvent
{
  [super removeLastObject];
}

-(unsigned int)numEvents
{
  return [super count];
}

-(unsigned long int)numBytes
{
  unsigned long int numBytes = 0;
  if([self resetIterator])
    do{
      numBytes += [[self currentObject] numBytes];
    }while([self advanceIterator: 1]);
  return numBytes;
}

-(unsigned char*)createContentsArrayOfSize: (unsigned long int*)numBytes
{
  unsigned long int bufferOffset = 0;
  *numBytes = [self numBytes];
  unsigned char* data = (unsigned char*)malloc(*numBytes);
  
  if([self resetIterator])
    {
      do{
	bufferOffset += [[self currentObject] putEventIntoBuffer: data withOffset: bufferOffset];
      }while([self advanceIterator: 1]);
    }

  return data;
}

-(float)durationInQuartersWithFramesPerBeat:(unsigned short int)frames
{
  float duration = 0;
  if([self resetIterator])
    {
      do{
	duration += [[self currentObject] deltaTimeAsNumQuartersWithFramesPerBeat: frames];
      }while([self advanceIterator: 1]);
    }
  return duration;
}

-(unsigned char)shortestNoteWithFramesPerBeat:(unsigned short int)frames
{
  unsigned char shortestNote=0;
  float currentNote;
  if([self resetIterator])
    {
      do{
	currentNote = [[self currentObject]  deltaTimeAsNumQuartersWithFramesPerBeat: frames];
	if(currentNote == 0) continue;
	currentNote = 4/(float)currentNote;
	if(currentNote > shortestNote) shortestNote = currentNote;
      }while([self advanceIterator: 1]);
    }
  return shortestNote;
}

-(void)quantizeToDuration:(unsigned char) duration withFramesPerBeat: (unsigned short)frames
{
  float deltaTime;
  double cumulativeDeltaTime = 0, quantizedCumulativeDeltaTime = 0;
  
  if([self resetIterator])
    {
      do{
	cumulativeDeltaTime += [[self currentObject] deltaTimeAsNumQuartersWithFramesPerBeat: frames];
	deltaTime = (float)cumulativeDeltaTime;
	deltaTime *= ((float)duration/4);
	deltaTime += (  2/(float)duration  );
	deltaTime = (unsigned long int)deltaTime;
	deltaTime -= quantizedCumulativeDeltaTime;
	quantizedCumulativeDeltaTime += deltaTime;
	deltaTime *= 4;
	deltaTime /= (float)duration;
	[[self currentObject] setDeltaTimeWithNumQuarters: deltaTime framesPerBeat: frames];
      }while([self advanceIterator: 1]);
    }
}

-(BOOL)containsNotes
{
  BOOL result = NO;
  if([self resetIterator])
    {
      do{
	if(([[self currentObject] statusByte] & 0xF0) == NOTE_ON)
	  {
	    result = YES;
	    break;
	  }
      }while([self advanceIterator: 1]);
    }
  return result;
}

-(id)free
{
  return [super free];
}

@end
