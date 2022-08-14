/*
 *  Synth.h
 *  Make weird noises
 *
 *  Made by Michael Krzyzaniak at Arizona State University's
 *  School of Arts, Media + Engineering in Spring of 2013
 *  mkrzyzan@asu.edu
 */

#include "Microphone.h"


/*OpaqueMicrophoneStruct----------------------------------*/
struct OpaqueMicrophoneStruct
{
  AUDIO_GUTS                  ;
  MKAiff*                      audioFile ;
};

int micAudioCallback         (void* SELF, auSample_t* buffer, int numFrames, int numChannels);

Microphone* micDestroy       (Microphone* self);


/*syNew---------------------------------------------------*/
Microphone* micNew(unsigned numChannels, int expectedSeconds)
{
  Microphone* self = (Microphone*) auAlloc(sizeof(*self), micAudioCallback, NO, numChannels);
  
  if(self != NULL)
    {
      self->destroy = (Audio* (*)(Audio*))micDestroy;
      self->audioFile = aiffWithDurationInSeconds(numChannels, AU_SAMPLE_RATE, 16, expectedSeconds + 5);
      if(self->audioFile == NULL)
        return (Microphone*)auDestroy((Audio*)self);
    }
  return self;
}

/*syDestroy-----------------------------------------------*/
MKAiff* micGetRecording(Microphone* self)
{
  return self->audioFile;
}

/*syDestroy-----------------------------------------------*/
Microphone* micDestroy(Microphone* self)
{
  if(self != NULL)
    {
      self->audioFile = aiffDestroy(self->audioFile);
    }
    
  return (Microphone*) NULL;
}

/*micAudioCallback-----------------------------------------*/
int micAudioCallback(void* SELF, auSample_t* buffer, int numFrames, int numChannels)
{
  Microphone* self = (Microphone*)SELF;
  if(!self->buffer_timestamp_is_supported) return 0; //extra invalid buffers come after stopping
  //fprintf(stderr, "time %llu, isPlaying %i\r\n", self->current_buffer_timestamp, self->isPlaying);
  
  return aiffAddFloatingPointSamplesAtPlayhead(self->audioFile, buffer, numFrames*numChannels, aiffFloatSampleType, aiffYes);
}

