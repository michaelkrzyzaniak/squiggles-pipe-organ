/*
 *  Synth.h
 *  Make weird noises
 *
 *  Made by Michael Krzyzaniak at Arizona State University's
 *  School of Arts, Media + Engineering in Spring of 2013
 *  mkrzyzan@asu.edu
 */

#ifndef __SYNTH2__
#define __SYNTH2__ 1

#if defined(__cplusplus)
extern "C"{
#endif   //(__cplusplus)

#include "AudioSuperclass.h"

typedef struct OpaqueSynth2Struct Synth2;

Synth2*  sy2New                   (            );
int      sy2IsDone                (Synth2* self);

#if defined(__cplusplus)
}
#endif   //(__cplusplus)

#endif   // __SYNTH2__
