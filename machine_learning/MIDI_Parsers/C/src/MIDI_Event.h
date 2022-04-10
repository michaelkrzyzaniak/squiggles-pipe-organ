#include "MIDI_Parser.h"

#ifndef __MK_MIDI_EVENT__
#define __MK_MIDI_EVENT__

#if defined(__cplusplus)
extern "C"{
#endif   //(__cplusplus)

typedef enum
{
  MIDI_EVENT_NOTE_NUMBER_ARGUMENT_INDEX = 0,
  MIDI_EVENT_VELOCITY_ARGUMENT_INDEX = 1,
}midi_event_argument_index_t;

typedef struct opaque_midi_event_struct MIDI_Event;

/* delta is the elapsed time since previous event */

MIDI_Event*    midi_event_new_with_delta_in_frames(unsigned frames, midi_status_t status, uint8_t* args, unsigned num_args);
//MIDI_Event*    midi_event_new_with_delta_in_beats (double beats, unsigned frames_per_beat, midi_status_t status, uint8_t* args, unsigned num_args);
MIDI_Event*    midi_event_shallow_copy(MIDI_Event* event);
MIDI_Event*    midi_event_deep_copy(MIDI_Event* event);

void           midi_event_set_delta_in_raw_bytes  (MIDI_Event* self, uint8_t* bytes, int num_bytes);
void           midi_event_set_delta_in_frames     (MIDI_Event* self, unsigned num_frames);
//void           midi_event_set_delta_in_beats      (MIDI_Event* self, double num_beats, unsigned frames_per_beat);

unsigned       midi_event_get_delta_in_raw_bytes  (MIDI_Event* self, uint8_t bytes[4]); //returns actual number of bytes, not to excede 4
unsigned       midi_event_get_delta_in_frames     (MIDI_Event* self);
//double         midi_event_get_delta_in_beats      (MIDI_Event* self, unsigned frames_per_beat);

void           midi_event_set_status_byte         (MIDI_Event* self, midi_status_t status);
void           midi_event_set_status_nibble       (MIDI_Event* self, midi_status_t status);
void           midi_event_set_channel             (MIDI_Event* self, midi_channel_t channel);
midi_status_t  midi_event_get_status_byte         (MIDI_Event* self);
midi_status_t  midi_event_get_status_nibble       (MIDI_Event* self);
midi_channel_t midi_event_get_channel             (MIDI_Event* self);

unsigned       midi_event_get_num_arguments       (MIDI_Event* self);
uint8_t*       midi_event_get_arguments           (MIDI_Event* self);
int            midi_event_get_argument_at_index   (MIDI_Event* self, midi_event_argument_index_t index); //returns -1 on error
void           midi_event_set_argument_at_index   (MIDI_Event* self, uint8_t argument, midi_event_argument_index_t index);

MIDI_Event*    midi_event_get_partner             (MIDI_Event* self); /* retunrs corresponding note off for note on and vice versa, or NULL */
void           midi_event_set_partner             (MIDI_Event* self, MIDI_Event* event);

unsigned       midi_event_get_num_bytes           (MIDI_Event* self);
unsigned       midi_event_serialize               (MIDI_Event* self, uint8_t* buffer, unsigned buffer_size); //returns numBytes writen;

MIDI_Event*    midi_event_destroy                 (MIDI_Event* self);

#if defined(__cplusplus)
}
#endif   //(__cplusplus)

#endif   //__MK_MIDI_EVENT__

