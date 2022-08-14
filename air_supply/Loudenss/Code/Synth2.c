/*
 *  Synth.h
 *  Make weird noises
 *
 *  Made by Michael Krzyzaniak at Arizona State University's
 *  School of Arts, Media + Engineering in Spring of 2013
 *  mkrzyzan@asu.edu
 */

#include "Synth2.h"
#include "stdio.h"

#define SY2_PULSE_TIME_SECONDS 0.0
#define SY2_PULSE_TIME_SAMPLES 1

#define SY2_LEAD_TIME_SECONDS  0.1
#define SY2_TAIL_TIME_SECONDS  0.9



#define SY2_LEAD_TIME_SAMPLES SY2_LEAD_TIME_SECONDS * AU_SAMPLE_RATE
#define SY2_TAIL_TIME_SAMPLES SY2_TAIL_TIME_SECONDS * AU_SAMPLE_RATE

int sy2AudioCallback(void* SELF, auSample_t* buffer, int numFrames, int numChannels);
Synth2*  sy2Destroy               (Synth2* self);

/*OpaqueSynthStruct----------------------------------------*/
struct OpaqueSynth2Struct
{
  AUDIO_GUTS        ;
  uint64_t sample_count;
  int isDone;
};

/*syNew---------------------------------------------------*/
Synth2* sy2New()
{
  Synth2* self = (Synth2*) auAlloc(sizeof(*self), sy2AudioCallback, YES, 1);
  if(self != NULL)
    {
      self->destroy = (Audio* (*)(Audio*))sy2Destroy;
    }
  return self;
}


/*syDestroy-----------------------------------------------*/
Synth2* sy2Destroy(Synth2* self)
{
  if(self != NULL)
    {
      
    }
  return (Synth2*)auDestroy((Audio*)self);
}

/*syAudioCallback-----------------------------------------*/
int     sy2IsDone                (Synth2* self)
{
  return self->isDone;
}

/*syAudioCallback-----------------------------------------*/
int sy2AudioCallback(void* SELF, auSample_t* buffer, int numFrames, int numChannels)
{
  Synth2* self = (Synth2*)SELF;
  int i;
  for(i=0; i<numFrames; i++)
    {
      if(self->sample_count < SY2_LEAD_TIME_SAMPLES)
        *buffer = 0;
      else if(self->sample_count < SY2_LEAD_TIME_SAMPLES  + SY2_PULSE_TIME_SAMPLES)
        *buffer = 1;
      else if(self->sample_count < SY2_LEAD_TIME_SAMPLES  + SY2_PULSE_TIME_SAMPLES + SY2_TAIL_TIME_SAMPLES)
        *buffer = 0;
      else
        {
          *buffer = 0;
          self->isDone = 1;
        }

      ++self->sample_count;
      buffer += numChannels;
    }
  
  return numFrames;
}


