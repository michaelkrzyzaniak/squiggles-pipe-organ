/*
 *  Synth.h
 *  Make weird noises
 *
 *  Made by Michael Krzyzaniak at Arizona State University's
 *  School of Arts, Media + Engineering in Spring of 2013
 *  mkrzyzan@asu.edu
 */

#ifndef __MICROPHONE__
#define __MICROPHONE__ 1

#if defined(__cplusplus)
extern "C"{
#endif   //(__cplusplus)

#include "AudioSuperclass.h"
#include "MKAiff.h"

typedef struct OpaqueMicrophoneStruct Microphone;

Microphone*       micNew               (unsigned numChannels, int expectedSeconds);
MKAiff*           micGetRecording      (Microphone* self);

//Microphone*  micDestroy             (Microphone*      self      );
//call with self->destroy(self);

#if defined(__cplusplus)
}
#endif   //(__cplusplus)

#endif   // __MICROPHONE__
