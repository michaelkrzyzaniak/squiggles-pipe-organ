/*
 *  Synth.h
 *  Make weird noises
 *
 *  Made by Michael Krzyzaniak at Arizona State University's
 *  School of Arts, Media + Engineering in Spring of 2013
 *  mkrzyzan@asu.edu
 */

#include "Synth.h"
#include "stdio.h"

#define SY_SET_FREQ(self, f) self->freq = ((f)*2*M_PI) * (1/AU_SAMPLE_RATE)
#define SY_NYQUIST_FREQ M_PI
#define SY_SECS_PER_OCTAVE 2

int syAudioCallback(void* SELF, auSample_t* buffer, int numFrames, int numChannels);
Synth*  syDestroy               (Synth* self);

/*OpaqueSynthStruct----------------------------------------*/
struct OpaqueSynthStruct
{
  AUDIO_GUTS        ;
  double            freq;
  double            phase;
  long double       freqIncrement;
  int               isDone;
};

/*syNew---------------------------------------------------*/
Synth* syNew()
{
  Synth* self = (Synth*) auAlloc(sizeof(*self), syAudioCallback, YES, 1);
  if(self != NULL)
    {
      self->destroy = (Audio* (*)(Audio*))syDestroy;
      self->phase = 0;
      SY_SET_FREQ(self, 10);
      self->freqIncrement = powl(2, 1 / (AU_SAMPLE_RATE * SY_SECS_PER_OCTAVE));
      self->isDone = 0;
    }
  return self;
}


/*syDestroy-----------------------------------------------*/
Synth* syDestroy(Synth* self)
{
  if(self != NULL)
    {
      
    }
  return (Synth*)auDestroy((Audio*)self);
}

/*syAudioCallback-----------------------------------------*/
int     syIsDone                (Synth* self)
{
  return self->isDone;
}

/*syAudioCallback-----------------------------------------*/
int syAudioCallback(void* SELF, auSample_t* buffer, int numFrames, int numChannels)
{
  Synth* self = (Synth*)SELF;
  int i;
  for(i=0; i<numFrames; i++)
    {
      if(self->freq <= SY_NYQUIST_FREQ  )
        {
          *buffer = sin(self->phase);
          self->phase += self->freq;
          self->freq *= self->freqIncrement;
        }
      else
        {
          *buffer = 0;
          self->isDone = 1;
        }
      buffer += numChannels;
    }
  
  return numFrames;
}


