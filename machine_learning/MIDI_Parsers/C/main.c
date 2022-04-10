//gcc *.c src/*.c

#include "src/MIDI_File.h"

void         midi_track_test_print_key_signatures(MIDI_Track* self, int tracknum);

int main(int argc, char* argv[])
{
  if(argc < 2)
    {fprintf(stderr, "which MIDI file(s) whould you like to process?\r\n"); exit(-1);}
  while(--argc>0)
    {
      argv++;
      MIDI_File* midi = midi_file_new_from_file(*argv);
      if(midi == NULL)
        {perror("Unable to open midi file"); return -1;}
      
      int   num_tracks = midi_file_get_num_tracks(midi); //assume first track is metadata
      fprintf(stderr, "num_tracks: %i\r\n", num_tracks);
      //int   frames_per_beat = midi_file_get_frames_per_beat(midi);
      List* tracks  = midi_file_get_track_list(midi);
      list_remove_data_at_index(tracks, 0, 1);
      
      MIDI_Track* track = midi_file_get_track_at_index(midi, 0);
      List* l = midi_track_get_event_list(track);
      MIDI_Event* event;
      list_iterator_t i = list_reset_iterator(l);
      while((event = list_advance_iterator(l, &i)) != NULL)
        {
          uint8_t nibble = midi_event_get_status_nibble(event);
          if((nibble != MIDI_STATUS_NOTE_ON) && (nibble != MIDI_STATUS_NOTE_OFF))
            list_remove_data(l, event, 1);
        }
      
      midi_file_save_with_filename(midi, "Stripped_Bach.mid");
    }

  return 0;
}
