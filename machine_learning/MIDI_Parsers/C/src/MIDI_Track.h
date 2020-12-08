#include <stdlib.h>

#import "List.h"
#import "MIDI_Event.h"

#ifndef __MK_MIDI_TRACK__
#define __MK_MIDI_TRACK__

#if defined(__cplusplus)
extern "C"{
#endif   //(__cplusplus)

typedef struct opaque_midi_track_struct MIDI_Track;

MIDI_Track*  midi_track_new();
MIDI_Track*  midi_track_deep_copy             (MIDI_Track* self);
void         midi_track_append_event          (MIDI_Track* self, MIDI_Event* event);
unsigned     midi_track_num_events            (MIDI_Track* self);
unsigned     midi_track_num_bytes             (MIDI_Track* self);
unsigned     midi_track_duration_in_frames    (MIDI_Track* self);
/* anything that relies on frames_per_beat should be in MIDI_File, not MIDI_Track or MIDI_Event */
//double       midi_track_duration_in_beats     (MIDI_Track* self, unsigned frames_per_beat);
//void         midi_track_grid_quantize         (MIDI_Track* self, double grid_size_beats, unsigned frames_per_beat);
//double       midi_track_shortest_note_in_beats(MIDI_Track* self, unsigned frames_per_beat);
BOOL         midi_track_contains_notes        (MIDI_Track* self); //some tracks contain only metadata
unsigned     midi_track_serialize             (MIDI_Track* self, uint8_t* buffer, unsigned buffer_size); //returns the number of bytes written
List*        midi_track_get_event_list        (MIDI_Track* self);

void         midi_track_insert_event_at_frame (MIDI_Track* self, MIDI_Event* event, unsigned frame, BOOL should_expand);
void         midi_track_remove_event          (MIDI_Track* self, MIDI_Event* event, BOOL should_collapse, BOOL should_deallocate);
unsigned     midi_track_get_frame_of_event    (MIDI_Track* self, MIDI_Event* event);
void         midi_track_merge_time_selection  (MIDI_Track* recipient, MIDI_Track* donor, unsigned start_frame , unsigned end_frame, BOOL should_remove_from_donor);
void         midi_track_merge                 (MIDI_Track* recipient, MIDI_Track* donor, BOOL should_remove_from_donor);
void         midi_track_transpose             (MIDI_Track* self, int semitones);

void         midi_track_attempt_to_resolve_note_on_off_pairs(MIDI_Track* self);

/* you give me the list, I fill it with note_on events. Don't destroy the events. appends to events without clearing */
void midi_track_get_active_notes_at_frame(MIDI_Track* self, unsigned frame, List* events);


MIDI_Track*  midi_track_destroy               (MIDI_Track* self);

#if defined(__cplusplus)
}
#endif   //(__cplusplus)

#endif   //__MK_MIDI_TRACK__
