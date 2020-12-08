#import "MIDI_Event.h"
#include <math.h>

//#include <stdio.h>//testing only

struct opaque_midi_event_struct
{
  unsigned       delta_in_frames;
  uint8_t        status_byte;
  uint8_t*       arguments;
  unsigned       num_arguments;
  MIDI_Event*    partner;
};
#include <stdio.h>
/*-------------------------------------------------------------------------------------*/
MIDI_Event*    midi_event_new_with_delta_in_frames(unsigned frames, midi_status_t status, uint8_t* args, unsigned num_args)
{
  MIDI_Event* self = calloc(1, sizeof(*self));
  
  if(self != NULL)
    {
      self->num_arguments = num_args;
      self->arguments   = calloc(num_args, 1);
      if(self->arguments == NULL)
        return midi_event_destroy(self);
        
      midi_event_set_delta_in_frames(self, frames);
      midi_event_set_status_byte(self, status);
  
      int i;
      for(i=0; i<num_args; i++)
        self->arguments[i] = args[i];
      
      //fix note off messages
      if(midi_event_get_status_nibble(self) == MIDI_STATUS_NOTE_ON)
        if(midi_event_get_argument_at_index(self, MIDI_EVENT_VELOCITY_ARGUMENT_INDEX) == 0)
          self->status_byte += 0x10;
    }
  
  return self;
}

/*-------------------------------------------------------------------------------------*/
/*
MIDI_Event*    midi_event_new_with_delta_in_beats (double beats, unsigned frames_per_beat, midi_status_t status, uint8_t* args, unsigned num_args)
{
  MIDI_Event* self = midi_event_new_with_delta_in_frames(0, status, args, num_args);
  if(self != NULL)
    midi_event_set_delta_in_beats(self, beats, frames_per_beat);
    
  return self;
}
*/

/*-------------------------------------------------------------------------------------*/
MIDI_Event*    midi_event_shallow_copy(MIDI_Event* event)
{
  MIDI_Event* self = midi_event_new_with_delta_in_frames(event->delta_in_frames, event->status_byte, event->arguments, event->num_arguments);
  if(self == NULL) return self;
  self->partner = NULL;
  return self;
}

/*-------------------------------------------------------------------------------------*/
MIDI_Event*    midi_event_deep_copy(MIDI_Event* event)
{
  MIDI_Event* self = midi_event_shallow_copy(event);
  if(self == NULL) return self;
  
  if(event->partner != NULL)
    {
      self->partner = midi_event_shallow_copy(event->partner);
      if(self->partner != NULL)
        self->partner->partner = self;
    }
  return self;
}

/*-------------------------------------------------------------------------------------*/
void  midi_event_set_delta_in_raw_bytes (MIDI_Event* self, uint8_t* bytes, int num_bytes)
{
  if(num_bytes>4) return;
  
  int i;
  unsigned num_frames = 0;
  for(i=0; i<num_bytes; i++)
    {
      num_frames <<= 7;
      num_frames |= bytes[i] & 0x7F;
    }
    
  midi_event_set_delta_in_frames(self, num_frames);
}

/*-------------------------------------------------------------------------------------*/
void midi_event_set_delta_in_frames  (MIDI_Event* self, unsigned num_frames)
{
  if(num_frames > 0xFFFFFFF) num_frames = 0xFFFFFFF;
  self->delta_in_frames = num_frames;
}

/*-------------------------------------------------------------------------------------*/
/*
void midi_event_set_delta_in_beats     (MIDI_Event* self, double num_beats, unsigned frames_per_beat)
{
  midi_event_set_delta_in_frames(self, lround(num_beats * frames_per_beat));
}
*/

/*-------------------------------------------------------------------------------------*/
unsigned midi_event_get_num_raw_delta_bytes(MIDI_Event* self)
{
  unsigned num_bytes;
  
  if(self->delta_in_frames > 0x1FFFFF)
    num_bytes = 4;
  else if(self->delta_in_frames > 0x3FFF)
    num_bytes = 3;
  else if(self->delta_in_frames > 0x7F)
    num_bytes = 2;
  else
    num_bytes = 1;
  
  return num_bytes;
}

/*-------------------------------------------------------------------------------------*/
unsigned midi_event_get_delta_in_raw_bytes(MIDI_Event* self, uint8_t bytes[4])
{
  unsigned delta_in_frames = self->delta_in_frames;
  unsigned num_bytes = midi_event_get_num_raw_delta_bytes(self);
  int i;

  bytes[num_bytes-1] = (delta_in_frames & 0x7F);
  
  for(i=1; i<num_bytes; i++)
    {
      delta_in_frames >>= 7;
      bytes[(num_bytes-1)-i] = ((delta_in_frames & 0x7F) | 0x80);
    }
  
  return num_bytes;
}

/*-------------------------------------------------------------------------------------*/
unsigned       midi_event_get_delta_in_frames    (MIDI_Event* self)
{
  return self->delta_in_frames;
}

/*-------------------------------------------------------------------------------------*/
/*
double          midi_event_get_delta_in_beats  (MIDI_Event* self, unsigned frames_per_beat)
{
  unsigned delta_in_frames = midi_event_get_delta_in_frames(self);
  return ((double)delta_in_frames) / ((double)frames_per_beat);
}
*/

/*-------------------------------------------------------------------------------------*/
midi_status_t  midi_event_get_status_byte                 (MIDI_Event* self)
{return self->status_byte;}

/*-------------------------------------------------------------------------------------*/
midi_status_t  midi_event_get_status_nibble               (MIDI_Event* self)
{return self->status_byte & 0xF0;}

/*-------------------------------------------------------------------------------------*/
midi_channel_t midi_event_get_channel                     (MIDI_Event* self)
{return self->status_byte & 0x0F;}

/*-------------------------------------------------------------------------------------*/
void           midi_event_set_status_byte                 (MIDI_Event* self, midi_status_t status)
{self->status_byte = status;}

/*-------------------------------------------------------------------------------------*/
void           midi_event_set_status_nibble               (MIDI_Event* self, midi_status_t status)
{
  self->status_byte &= 0x0F;
  status &= 0xF0;
  self->status_byte |= status;
}

/*-------------------------------------------------------------------------------------*/
void           midi_event_set_channel                     (MIDI_Event* self, midi_channel_t channel)
{
  self->status_byte &= 0xF0;
  channel &= 0x0F;
  self->status_byte |= channel;
}

/*-------------------------------------------------------------------------------------*/
unsigned       midi_event_get_num_arguments         (MIDI_Event* self)
{ return self->num_arguments;}

/*-------------------------------------------------------------------------------------*/
uint8_t* midi_event_get_arguments (MIDI_Event* self)
{return self->arguments;}

/*-------------------------------------------------------------------------------------*/
int midi_event_get_argument_at_index           (MIDI_Event* self, midi_event_argument_index_t index)
{
  if(index < self->num_arguments)
    return self->arguments[index];
  else return -1;
}

/*-------------------------------------------------------------------------------------*/
void           midi_event_set_argument_at_index           (MIDI_Event* self, uint8_t argument, midi_event_argument_index_t index)
{
  if(index < self->num_arguments)
    self->arguments[index] = argument;
}

/*-------------------------------------------------------------------------------------*/
MIDI_Event*    midi_event_get_partner             (MIDI_Event* self)
{
  return (MIDI_Event*)self->partner;
}

/*-------------------------------------------------------------------------------------*/
void           midi_event_set_partner             (MIDI_Event* self, MIDI_Event* event)
{
  self->partner = (MIDI_Event*)event;
}

/*-------------------------------------------------------------------------------------*/
unsigned       midi_event_get_num_bytes                   (MIDI_Event* self)
{
  return midi_event_get_num_raw_delta_bytes(self) + 1 /*status byte*/ + self->num_arguments;
}

/*-------------------------------------------------------------------------------------*/
unsigned       midi_event_serialize                       (MIDI_Event* self, uint8_t* buffer, unsigned buffer_size)
{
  int event_num_bytes = midi_event_get_num_bytes(self);
  
  if(buffer_size < event_num_bytes)
    return 0;
  
  uint8_t delta_time[4];
  unsigned n = midi_event_get_delta_in_raw_bytes(self, delta_time);

  int i;
  for(i=0; i<n; i++)
    *buffer++ = delta_time[i];
    
  *buffer++ = self->status_byte;
  
  for(i=0; i<self->num_arguments; i++)
    *buffer++ = self->arguments[i];
  
  return event_num_bytes;
}

/*-------------------------------------------------------------------------------------*/
MIDI_Event*    midi_event_destroy                         (MIDI_Event* self)
{
  if(self != NULL)
    {
      if(self->arguments != NULL)
        free(self->arguments);
      
      free(self);
    }
  return (MIDI_Event*) NULL;
}
