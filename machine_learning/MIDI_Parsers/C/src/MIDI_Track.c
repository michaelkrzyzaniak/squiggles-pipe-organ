#include "MIDI_Track.h"
#include "stdio.h" //testing olny

struct opaque_midi_track_struct
{
  List* list;
};

/*-------------------------------------------------------------------------------------*/
MIDI_Track* midi_track_new()
{
  MIDI_Track* self = calloc(1, sizeof(*self));
  if(self != NULL)
    {
      self->list = list_new();
      if(self->list == NULL)
        return midi_track_destroy(self);
    }
  return self;
}

/*-------------------------------------------------------------------------------------*/
MIDI_Track*  midi_track_deep_copy(MIDI_Track* self)
{
  MIDI_Track* copy = midi_track_new();
  if(copy != NULL)
    {
      List* l = midi_track_get_event_list(self);
      MIDI_Event* event;
      list_iterator_t i = list_reset_iterator(self->list);
      while((event = list_advance_iterator(self->list, &i)) != NULL)
        {
          MIDI_Event* copied_event = midi_event_deep_copy(event);
          if(event != NULL)
            midi_track_append_event(copy, copied_event);
          else
            return midi_track_destroy(copy);
        }
      midi_track_attempt_to_resolve_note_on_off_pairs(copy);
    }
  return copy;
}

/*-------------------------------------------------------------------------------------*/
void midi_track_append_event(MIDI_Track* self, MIDI_Event* event)
{
  list_append_data(self->list, event, (list_data_deallocator_t)midi_event_destroy);
}

/*-------------------------------------------------------------------------------------*/
unsigned midi_track_num_events(MIDI_Track* self)
{
  return list_count(self->list);
}

/*-------------------------------------------------------------------------------------*/
unsigned midi_track_num_bytes(MIDI_Track* self)
{
  unsigned num_bytes = 0;
  MIDI_Event* event;
  list_iterator_t i = list_reset_iterator(self->list);
  while((event = list_advance_iterator(self->list, &i)) != NULL)
    num_bytes += midi_event_get_num_bytes(event);

  return num_bytes;
}

/*-------------------------------------------------------------------------------------*/
unsigned midi_track_serialize(MIDI_Track* self, uint8_t* buffer, unsigned buffer_size)
{
  unsigned buffer_offset = 0;
  unsigned num_bytes = midi_track_num_bytes(self);
  if(num_bytes < buffer_size)
    return 0;
  
  MIDI_Event* event;
  list_iterator_t i = list_reset_iterator(self->list);
  while((event = list_advance_iterator(self->list, &i)) != NULL)
    buffer_offset += midi_event_serialize(event, buffer+buffer_offset, buffer_size-buffer_offset);

  return buffer_offset;
}

/*-------------------------------------------------------------------------------------*/
unsigned midi_track_duration_in_frames(MIDI_Track* self)
{
  double frames = 0;
  
  unsigned num_bytes = 0;
  MIDI_Event* event;
  list_iterator_t i = list_reset_iterator(self->list);
  while((event = list_advance_iterator(self->list, &i)) != NULL)
    frames += midi_event_get_delta_in_frames(event);
  
  return frames;
}

/*-------------------------------------------------------------------------------------*/
/*
double    midi_track_duration_in_beats(MIDI_Track* self, unsigned frames_per_beat)
{
  return (double)(midi_track_duration_in_frames(self)) / ((double)frames_per_beat);
}
*/


/*-------------------------------------------------------------------------------------*/
/*
double    midi_track_shortest_note_in_beats(MIDI_Track* self, unsigned frames_per_beat)
{
  unsigned shortest_note = 0;
  double current_note;
  
  MIDI_Event* event;
  list_iterator_t i = list_reset_iterator(self->list);
  while((event = list_advance_iterator(self->list, &i)) != NULL)
    {
      //todo: check that the event is a note?
      //compare note_on to note_off time?
      current_note = midi_event_get_delta_in_beats(event, frames_per_beat);
      if(current_note == 0) continue;
      //what? Why????????????????????????????????????????????????
      //current_note = 4/(double)current_note;
      //if(current_note > shortest_note)
        //shortest_note = current_note;
      if(current_note < shortest_note)
        shortest_note = current_note;
    }
  return shortest_note;
}
*/

/*-------------------------------------------------------------------------------------*/
/*
void     midi_track_grid_quantize(MIDI_Track* self, double grid_size_beats, unsigned frames_per_beat)
{
  double duration;
  double cumulative_duration = 0;
  double quantized_cumulative_duration = 0;

  MIDI_Event* event;
  list_iterator_t i = list_reset_iterator(self->list);
  while((event = list_advance_iterator(self->list, &i)) != NULL)
    {
	    cumulative_duration += midi_event_get_delta_in_beats(event, frames_per_beat);
	    duration = (double)cumulative_duration;
	    duration *= ((double)grid_size_beats/4);
	    duration += ( 2/(double)grid_size_beats );
	    duration = (unsigned long int)duration;
	    duration -= quantized_cumulative_duration;
	    quantized_cumulative_duration += duration;
      duration *= 4;
	    duration /= (double)grid_size_beats;
      midi_event_set_delta_in_beats(event, duration, frames_per_beat);
    }
}
*/

/*-------------------------------------------------------------------------------------*/
BOOL      midi_track_contains_notes(MIDI_Track* self)
{
  BOOL result = NO;
  
  MIDI_Event* event;
  list_iterator_t i = list_reset_iterator(self->list);
  while((event = list_advance_iterator(self->list, &i)) != NULL)
    {
	    if(midi_event_get_status_nibble(event) == MIDI_STATUS_NOTE_ON)
        {
          result = YES;
	        break;
        }
    }
    
  return result;
}

/*-------------------------------------------------------------------------------------*/
List* midi_track_get_event_list(MIDI_Track* self)
{
  return self->list;
}

/*-------------------------------------------------------------------------------------*/
void         midi_track_insert_event_at_frame (MIDI_Track* self, MIDI_Event* event, unsigned frame, BOOL should_expand)
{
  unsigned frame_count = 0;
  unsigned prev_frame_count = 0;
  unsigned next_delta;
  unsigned index=0;
  
  MIDI_Event* e;
  list_iterator_t i = list_reset_iterator(self->list);
  while((e = list_advance_iterator(self->list, &i)) != NULL)
    {
      next_delta = midi_event_get_delta_in_frames(e);
      frame_count += next_delta;
      if(frame_count >= frame)
        break;
      
      ++index;
      prev_frame_count = frame_count;
    }
  
  midi_event_set_delta_in_frames(event, frame-prev_frame_count);
  if(!should_expand)
    {
      if(e != NULL) /* there are more notes after this one */
        midi_event_set_delta_in_frames(e, next_delta-(frame-prev_frame_count));
    }
  list_insert_data_at_index(self->list, event, index, (list_data_deallocator_t)midi_event_destroy);
}

/*-------------------------------------------------------------------------------------*/
//todo: remove partner too
void         midi_track_remove_event          (MIDI_Track* self, MIDI_Event* event, BOOL should_collapse, BOOL should_deallocate)
{
  unsigned index_of_event;
  if(list_contains_data(self->list, event, &index_of_event))
    {
      if(!should_collapse)
        {
          unsigned count = list_count(self->list);
          if(count > (index_of_event+1))
            {
              MIDI_Event* next_event = list_data_at_index(self->list, index_of_event+1);
              unsigned delta = midi_event_get_delta_in_frames(event);
              delta += midi_event_get_delta_in_frames(next_event);
              midi_event_set_delta_in_frames(next_event, delta);
            }
        }
      list_remove_data(self->list, event, should_deallocate);
    }
}

/*-------------------------------------------------------------------------------------*/
unsigned     midi_track_get_frame_of_event    (MIDI_Track* self, MIDI_Event* event)
{
  unsigned frame_counter = 0;
  if(list_contains_data(self->list, event, NULL))
    {
      MIDI_Event* e;
      list_iterator_t i = list_reset_iterator(self->list);
      while((e = list_advance_iterator(self->list, &i)) != NULL)
        {
          frame_counter += midi_event_get_delta_in_frames(e);
          if(e == event)
            break;
        }
    }
  return frame_counter;
}

/*-------------------------------------------------------------------------------------*/
//inclusive of start and end frame
//this could orphan note_offs
void midi_track_merge_time_selection(MIDI_Track* recipient, MIDI_Track* donor, unsigned start_frame, unsigned end_frame, BOOL should_remove_from_donor)
{
  unsigned frame_counter = 0;
  MIDI_Event* event;
  list_iterator_t i = list_reset_iterator(donor->list);
  while((event = list_advance_iterator(donor->list, &i)) != NULL)
    {
      frame_counter = midi_track_get_frame_of_event(donor, event);
      if(frame_counter > end_frame)
        break;
      if(frame_counter >= start_frame)
        {
          MIDI_Event* event_copy = midi_event_shallow_copy(event);
          if(event_copy == NULL) break;
          midi_track_insert_event_at_frame(recipient, event_copy, frame_counter, NO);
          if(should_remove_from_donor)
            midi_track_remove_event(donor, event, NO, YES);
        }
    }
}

/*-------------------------------------------------------------------------------------*/
void midi_track_merge(MIDI_Track* recipient, MIDI_Track* donor, BOOL should_remove_from_donor)
{
  unsigned duration = midi_track_duration_in_frames(donor);
  midi_track_merge_time_selection(recipient, donor, 0,  duration, should_remove_from_donor);
}

/*-------------------------------------------------------------------------------------*/
//todo: change key if it exists;
void     midi_track_transpose(MIDI_Track* self, int semitones)
{
  if(semitones >  128) semitones =  128;
  if(semitones < -128) semitones = -128;
  
  unsigned frame_counter = 0;
  MIDI_Event* event;
  list_iterator_t i = list_reset_iterator(self->list);
  while((event = list_advance_iterator(self->list, &i)) != NULL)
    {
      int status = midi_event_get_status_nibble(event);
      if((status == MIDI_STATUS_NOTE_ON) || (status == MIDI_STATUS_NOTE_OFF))
        {
          int note = midi_event_get_argument_at_index(event, MIDI_EVENT_NOTE_NUMBER_ARGUMENT_INDEX);
          if(note != -1)
            {
              note += semitones;
              if(note < 0) note = 0;
              else if(note > 127) note = 127;
              midi_event_set_argument_at_index(event, (uint8_t)note, MIDI_EVENT_NOTE_NUMBER_ARGUMENT_INDEX);
            }
        }
      else if(midi_event_get_status_byte(event) == MIDI_STATUS_META_EVENT)
        {
          if(midi_event_get_num_arguments(event) != 4)
            continue;
          
          uint8_t* args = midi_event_get_arguments(event);
          
          if(args[0] != MIDI_META_KEY_SIGNATURE)
            continue;
         
          //args[1] == 2; //num bytes to follow
          signed char num_accidentals = args[2];
          //int mode = args[3]; //(0=MAJ, 1=MIN)
          
          while(num_accidentals < -5)
            num_accidentals += 12;
          while(num_accidentals > 6)
            num_accidentals -= 12;
          
          int map_index;
          int map[] = {0, -5, 2, -3, 4, -1, 6, 1, -4, 3, -2, 5};
          
          for(map_index=0; map_index<12; map_index++)
            if(map[map_index] == num_accidentals)
              break;
          
          if((map_index < 12) & (map_index >= 0))
            {
              map_index += semitones;
              while (map_index < 0)
                map_index += 12;
              while (map_index >= 12)
                map_index -= 12;
              num_accidentals = map[map_index];
              
              args[2] = num_accidentals;
            }
        }
    }
}

/*-------------------------------------------------------------------------------------*/
void         midi_track_test_print_key_signatures(MIDI_Track* self, int tracknum)
{
  MIDI_Event* event;
  list_iterator_t i = list_reset_iterator(self->list);
  while((event = list_advance_iterator(self->list, &i)) != NULL)
    {
      if(midi_event_get_status_byte(event) == MIDI_STATUS_META_EVENT)
        {
          if(midi_event_get_num_arguments(event) != 4)
            continue;
          
          uint8_t* args = midi_event_get_arguments(event);
          
          if(args[0] != MIDI_META_KEY_SIGNATURE)
            continue;
         
          //args[1] == 2; //num bytes to follow
          signed char num_accidentals = args[2];
          int mode = args[3]; //(0=MAJ, 1=MIN)
          
          fprintf(stderr, "track: %i\tnum_accidentals: %hhi\tmode:%i\r\n", tracknum, num_accidentals, mode);
        }
    }
}

/*-------------------------------------------------------------------------------------*/
void         midi_track_attempt_to_resolve_note_on_off_pairs(MIDI_Track* self)
{
  unsigned frame_counter=0;
  unsigned num_active_notes = 0;
  MIDI_Event* note_on;
  list_iterator_t i = list_reset_iterator(self->list);
  int index=0;
  int count = list_count(self->list);
  
  while((note_on = list_advance_iterator(self->list, &i)) != NULL)
    {
      if(midi_event_get_status_nibble(note_on) == MIDI_STATUS_NOTE_ON)
        {
          if(midi_event_get_partner(note_on) == NULL)
            {
              int on_note = midi_event_get_argument_at_index(note_on, MIDI_EVENT_NOTE_NUMBER_ARGUMENT_INDEX);
              int on_chan = midi_event_get_channel(note_on);
              int j;
              
              for(j=index+1; j<count; j++)
                {
                  MIDI_Event* note_off = (MIDI_Event*)list_data_at_index(self->list, j);
                  if(midi_event_get_status_nibble(note_off) == MIDI_STATUS_NOTE_OFF)
                    {
                      if(midi_event_get_partner(note_off) == NULL)
                        {
                          int off_note = midi_event_get_argument_at_index(note_off, MIDI_EVENT_NOTE_NUMBER_ARGUMENT_INDEX);
                          int off_chan = midi_event_get_channel(note_off);
                          if((on_note == off_note) && (on_chan == off_chan))
                            {
                              midi_event_set_partner(note_on, note_off);
                              midi_event_set_partner(note_off, note_on);
                              break;
                              
                            }
                        }
                    }
                }
            }
        }
      ++index;
    }

}

/*-------------------------------------------------------------------------------------*/
void midi_track_get_active_notes_at_frame(MIDI_Track* self, unsigned frame, List* events)
{
  midi_track_attempt_to_resolve_note_on_off_pairs(self);
  
  unsigned frame_counter=0;
  unsigned num_active_notes = 0;
  MIDI_Event* note_on;
  list_iterator_t i = list_reset_iterator(self->list);
  while((note_on = list_advance_iterator(self->list, &i)) != NULL)
    {
      frame_counter += midi_event_get_delta_in_frames(note_on);
      if(frame_counter > frame)
        break;
      
      if(midi_event_get_status_nibble(note_on) == MIDI_STATUS_NOTE_ON)
        {
          BOOL is_active = YES;
          MIDI_Event* note_off = midi_event_get_partner(note_on);
          if(note_off != NULL)
            {
              unsigned off_frame = midi_track_get_frame_of_event(self, note_off);
              if(off_frame <= frame)
                is_active = NO;
            }
          
          if(is_active)
            list_append_data(events, note_on, NULL);
        }
    }
}

/*-------------------------------------------------------------------------------------*/
MIDI_Track* midi_track_destroy(MIDI_Track* self)
{
  if(self != NULL)
    {
      list_destroy(self->list, YES);
      free(self);
    }
  return (MIDI_Track*) NULL;
}
