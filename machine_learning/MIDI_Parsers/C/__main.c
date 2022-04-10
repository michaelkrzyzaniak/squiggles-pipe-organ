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
      
      int   num_tracks = midi_file_get_num_tracks(midi) - 1; //assume first track is metadata
      int   frames_per_beat = midi_file_get_frames_per_beat(midi);
      List* tracks  = midi_file_get_track_list(midi);
      int   track_mask;
      int   max_track_mask = 1 << num_tracks;
      int   transpose = -6;
      int   max_transpose = 6;

      fprintf(stderr, "processing %s -- %i tracks -- %i frames per beat\r\n", *argv, num_tracks, frames_per_beat);
      
/*
      int tracknum = 0;
      MIDI_Track* track;
      list_iterator_t iter = list_reset_iterator(tracks);
      while((track = list_advance_iterator(tracks, &iter)) != NULL)
        {
          midi_track_test_print_key_signatures(track, tracknum);
          ++tracknum;
        }
      continue;
*/
      for(/*transpose*/; transpose<max_transpose; transpose++)
        {
          for(track_mask=1; track_mask<max_track_mask; track_mask++)
            {
              MIDI_File* new_midi = midi_file_new(frames_per_beat);
              if(new_midi == NULL)
                { perror("unable to make a new MIDI file"); return -1; }

              int i=0;
              MIDI_Track* track;
              list_iterator_t iter = list_reset_iterator(tracks);
              int mask = (track_mask << 1) + 1; //mask in the metadata track
              while((track = list_advance_iterator(tracks, &iter)) != NULL)
                {
                  if((mask >> i) & 0x01)
                    {
                      MIDI_Track* copied_track = midi_track_deep_copy(track);
                      if(copied_track != NULL)
                        {
                          midi_track_transpose(copied_track, transpose);
                          midi_file_append_track(new_midi, copied_track);
                        }
                      else
                        { perror("unable to make a new MIDI track"); return -1; }
                    }
                  ++i;
                }
              char* filename;
              asprintf(&filename, "dataset/%s_%i_%i.mid", *argv, transpose, track_mask);
              fprintf(stderr, "%s\r\n", filename);
              midi_file_save_with_filename(new_midi, filename);
              new_midi = midi_file_destroy(new_midi);
              free(filename);
            }
        }
      midi_file_destroy(midi);
    }

  return 0;
}
