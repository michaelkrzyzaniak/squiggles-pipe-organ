/*------------------------------------------------------------------------
 _  _                         _             _
| || |__ _ _ _ _ __  ___ _ _ (_)______ _ _ | |_
| __ / _` | '_| '  \/ _ \ ' \| |_ / -_) '_|| ' \
|_||_\__,_|_| |_|_|_\___/_||_|_/__\___|_|(_)_||_|
------------------------------------------------------------------------
Written by Michael Krzyzaniak

Version 2.0 March 21 2021
multi-thread support and many small changes
----------------------------------------------------------------------*/

#ifndef __HARMONIZER__
#define __HARMONIZER__
  
#if defined(__cplusplus)
extern "C"{
#endif   //(__cplusplus)

#include "Matrix.h"
#include "AudioSuperclass.h"

#include <stdio.h>

typedef void (*harmonizer_note_changed_callback_t)(void* SELF, int midi_note);

typedef struct opaque_harmonizer_struct Harmonizer;

Harmonizer* harmonizer_new                      (char* folder);
Harmonizer* harmonizer_destroy                  (Harmonizer* self);
void        harmonizer_set_note_changed_callback(Harmonizer* self, harmonizer_note_changed_callback_t callback,  void* callback_self);
void harmonizer_process_audio(Harmonizer* self, auSample_t* buffer, int num_frames);

void harmonizer_test_io(Harmonizer* self, matrix_val_t* input, matrix_val_t* output);

#if defined(__cplusplus)
}
#endif   //(__cplusplus)

#endif   //__MATRIX__