#include "Beep.h"

/*--------------------------------------------------------------------*/
struct OpaqueBeepStruct
{
  AUDIO_GUTS                  ;
  fastsin_t phase             ;
  fastsin_t freq              ;
};

/*--------------------------------------------------------------------*/
int   beep_audio_callback       (void* SELF, auSample_t* buffer, int num_frames, int num_channels);
Beep* beep_destroy              (Beep* self);


/*--------------------------------------------------------------------*/
Beep* beep_new()
{
  Beep* self = (Beep*) auAlloc(sizeof(*self), beep_audio_callback, YES, 1);
  
  if(self != NULL)
    {
      self->destroy = (Audio* (*)(Audio*))beep_destroy;
      beep_set_note(self, 0);
    }
  return self;
}


/*--------------------------------------------------------------------*/
Beep* beep_destroy(Beep* self)
{
  if(self != NULL)
    {

    }
    
  return (Beep*) NULL;
}

/*--------------------------------------------------------------------*/
void beep_set_note(Beep* self, float midi_note_number)
{
  self->freq = AU_MIDI2FREQ(midi_note_number);
  fprintf(stderr, "MIDI: %f, FREQ: %u\r\n", midi_note_number, self->freq);
}

/*--------------------------------------------------------------------*/
int beep_audio_callback(void* SELF, auSample_t* buffer, int num_frames, int num_channels)
{
  Beep* self = SELF;
  
  //memset(buffer, 0, sizeof(*buffer) * num_frames * num_channels);

  int frame, chan;
  auSample_t sample;

  

  for(frame=0; frame<num_frames; frame++)
    {
      sample = fastsin(self->phase);
      self->phase += self->freq;

      
      for(chan=0; chan<num_channels; chan++)
        *buffer++ = sample;
    }

  return  num_frames;
}

