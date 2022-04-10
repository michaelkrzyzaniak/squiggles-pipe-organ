
#import <stdlib.h>//&#60stdlib.h&gt
#import "MKArray.h"//&#60MKArray.h&gt
#import "MKMIDIEvent.h"

@interface MKMIDIEventList : MKArray
{

}

-(void)addEvent:(MKMIDIEvent*)event; 
-(void)removeLastEvent;
-(unsigned long int)numBytes;
-(unsigned char*)createContentsArrayOfSize:(unsigned long int*)numBytes; //the size will be written into *numBytes
-(float)durationInQuartersWithFramesPerBeat:(unsigned short int)frames;
-(void)quantizeToDuration:(unsigned char) duration withFramesPerBeat: (unsigned short)frames;
-(unsigned char)shortestNoteWithFramesPerBeat:(unsigned short int)frames;
-(BOOL)containsNotes; //some tracks contain only metadata


@end
