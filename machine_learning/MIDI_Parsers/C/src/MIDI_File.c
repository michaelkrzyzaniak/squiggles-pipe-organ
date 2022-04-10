#import "MIDI_File.h"

/*-------------------------------------------------------------------------------------*/
struct opaque_midi_file_struct
{
  List* track_list;
  uint16_t frames_per_beat;
};

/*-------------------------------------------------------------------------------------*/
typedef enum
  {
    MIDI_FILE_MONOPHONIC_FORMAT_TYPE = 0,
    MIDI_FILE_POLYPHONIC_FORMAT_TYPE = 1,
    MIDI_FILE_TYPE_THREE_FORMAT_TYPE = 2,
  }midi_file_format_type_t;

typedef enum
  {
    MIDI_FILE_HEADER_CHUNK_ID = 'MThd',
    MIDI_FILE_TRACK_CHUNK_ID = 'MTrk',
  }midi_file_chunk_identifier_t;

/*-------------------------------------------------------------------------------------*/
#define MIDI_FILE_HEADER_CHUNK_SIZE  6
typedef struct
{
  uint32_t  chunk_id;
  uint32_t  chunk_size;
  uint16_t  format_type;
  uint16_t  num_tracks;
  uint16_t  time_division;
}midi_file_header_chunk_t;

typedef struct
{
  uint32_t chunk_id;
  uint32_t chunk_size;
}midi_file_track_chunk_t;

/*-------------------------------------------------------------------------------------*/
BOOL midi_file_read_uint_32(FILE* file, uint32_t* returned_val)
{
  uint8_t b[4];
  
  if(fread(b, 1, 4, file) != 4)
    return NO;
  
  *returned_val = (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
  return YES;
}

/*-------------------------------------------------------------------------------------*/
BOOL midi_file_read_uint_16(FILE* file, uint16_t* returned_val)
{
  uint8_t b[2];
  
  if(fread(b, 1, 2, file) != 2)
    return NO;
  
  *returned_val = (b[0] << 8) |(b[1]);
  return YES;
}

/*-------------------------------------------------------------------------------------*/
BOOL midi_file_write_uint_32(FILE* file, uint32_t val)
{
  uint8_t b[4];
  
  b[0] = (val >> 24) & 0xFF;
  b[1] = (val >> 16) & 0xFF;
  b[2] = (val >>  8) & 0xFF;
  b[3] = (val)       & 0xFF;
  
  if(fwrite(b, 1, 4, file) != 4)
    return NO;
  
  return YES;
}

/*-------------------------------------------------------------------------------------*/
BOOL midi_file_write_uint_16(FILE* file, uint16_t val)
{
  uint8_t b[2];

  b[0] = (val >>  8) & 0xFF;
  b[1] = (val)       & 0xFF;
  
  if(fwrite(b, 1, 2, file) != 2)
    return NO;
  
  return YES;
}

/*-------------------------------------------------------------------------------------*/
BOOL midi_file_read_header(FILE* file, midi_file_header_chunk_t* returned_header)
{
  if(midi_file_read_uint_32(file, &returned_header->chunk_id) == NO)
    return NO;
  if(midi_file_read_uint_32(file, &returned_header->chunk_size) == NO)
    return NO;
  if(midi_file_read_uint_16(file, &returned_header->format_type) == NO)
    return NO;
  if(midi_file_read_uint_16(file, &returned_header->num_tracks) == NO)
    return NO;
  if(midi_file_read_uint_16(file, &returned_header->time_division) == NO)
    return NO;
    
  if(returned_header->chunk_id != MIDI_FILE_HEADER_CHUNK_ID)
    return NO;
  
  return YES;
}

/*-------------------------------------------------------------------------------------*/
BOOL midi_file_read_track(FILE* file, midi_file_track_chunk_t* returned_track)
{
  if(midi_file_read_uint_32(file, &returned_track->chunk_id) == NO)
    return NO;
  if(midi_file_read_uint_32(file, &returned_track->chunk_size) == NO)
    return NO;
  
  if(returned_track->chunk_id != MIDI_FILE_TRACK_CHUNK_ID)
    return NO;
    
  return YES;
}

/*-------------------------------------------------------------------------------------*/
//-(void)addString:(char*)string toTrack:(unsigned int)track withType:(mkMIDIMetaEvent_t)type;
//-(float)getPreviousRestOnTrack:(unsigned int)track;

void midi_file_remove_end_from_all_tracks(MIDI_File* self);
void midi_file_end_all_tracks(MIDI_File* self);

//-(void)setFramesPerBeat:(unsigned short int)value;
void midi_file_parse_track_data(MIDI_File* self, uint8_t* track_data, unsigned size);

void midi_file_add_byte_array_to_track(MIDI_File* self, MIDI_Track* track, uint8_t* bytes, unsigned num_bytes, double beats_delta);

/*-------------------------------------------------------------------------------------*/
/*
-(float)getPreviousRestOnTrack:(unsigned int)track
{
  float value = [previousRest floatAtIndex: track-1];
  [previousRest setFloat: 0.0 atIndex: track-1];
  return value;
}
*/
/*-------------------------------------------------------------------------------------*/
MIDI_File* midi_file_new_from_file(char* filename)
{
  FILE* file = fopen(filename, "r");
  
  if(file == NULL)
    return NULL;
    
  midi_file_header_chunk_t header;
  if(midi_file_read_header(file, &header) == NO)
    {fclose(file); return NULL;}
  
  MIDI_File* self = midi_file_new(header.time_division);

  if(self != NULL)
    {
      int i;
      for(i=0; i<header.num_tracks; i++)
        {
          midi_file_track_chunk_t track;
          if(midi_file_read_track(file, &track) == NO)
            {fclose(file); return midi_file_destroy(self);}

          uint8_t* track_data = malloc(track.chunk_size);
          if(track_data == NULL)
            {fclose(file); return midi_file_destroy(self);}
            
          fread(track_data, 1, track.chunk_size, file);
          midi_file_parse_track_data(self, track_data, track.chunk_size);
          free(track_data);
        }
        
      midi_file_remove_end_from_all_tracks(self);
    }
  
  fclose(file);
  return self;
}

/*-------------------------------------------------------------------------------------*/
void midi_file_parse_track_data(MIDI_File* self, uint8_t* track_data, unsigned size)
{
  MIDI_Track* track = midi_file_add_empty_tracks(self, 1);
  if(track == NULL) return;

  unsigned  num_bytes_read = 0;
  uint8_t   delta_time[4];
  unsigned  num_delta_bytes;
  unsigned  num_arguments;
  
  uint8_t   status;
  
  while(num_bytes_read < size)
    {
      num_delta_bytes = 0;
      
      //get delta time
      while(*track_data > 0x7F)
        {
          if(num_delta_bytes >= 3)
            return;
	        delta_time[num_delta_bytes++] = *track_data++;
	        num_bytes_read++;
        }
      
      //get last byte of delta time (first bit clear)
      delta_time[num_delta_bytes++] = *track_data++;
      num_bytes_read++;
    
      //get status byte
      if(*track_data >= 0x80)
        {
          status = *track_data++;
          num_bytes_read++;
        }

      //get arguments;
      if((status & 0xF0) == MIDI_STATUS_NOTE_ON)
	      num_arguments = 2;
      else if((status & 0xF0) == MIDI_STATUS_NOTE_OFF)
	      num_arguments = 2;
      else if((status & 0xF0) == MIDI_STATUS_POLYPHONIC_AFTERTOUCH)
	      num_arguments = 2;
      else if(status == MIDI_STATUS_PITCH_WHEEL)
	      num_arguments = 2;
      else if((status & 0xF0) == MIDI_STATUS_MODE_CHANGE)
	      num_arguments = 2;
      else if((status & 0xF0) == MIDI_STATUS_PROGRAM_CHANGE)
	      num_arguments = 1;
      else if((status & 0xF0) == MIDI_STATUS_CHANNEL_AFTERTOUCH)
	      num_arguments = 1;
      else if(status == MIDI_STATUS_META_EVENT)
        {
          //track_data[0] is meta event type
          //track_data[1] is first byte of message length
          //there may be up to 4 bytes in the message length, the last one
          //will have first bit clear
          int num_arguments_length = 1;
          if(track_data[1] > 0x7F)
            {
              num_arguments_length = 2;
              if(track_data[2] > 0x7F)
                {
                  num_arguments_length = 3;
                  if(track_data[3] > 0x7F)
                    {
                      num_arguments_length = 4;
                      if(track_data[4] > 0x7F)
                        continue;//some sort of error here;
                    }
                }
            }

          int i;
          num_arguments = 0;
          for(i=0; i<num_arguments_length; i++)
            {
               num_arguments <<= 7;
               num_arguments |= track_data[i+1] & 0x7F;
            }

          num_arguments += 1; //for the event type
          num_arguments += num_arguments_length; //for the message length bytes
          
         //VARIABLE LENGTH!
        }
      else if(status == MIDI_STATUS_SYSTEM_EXCLUSIVE)
	      {
	        num_arguments = 1;
	        while(track_data[num_arguments-1] != MIDI_STATUS_END_OF_EXCLUSIVE)
	          {num_arguments++;}
	      }
      else continue;

      //if((status == MIDI_STATUS_NOTE_ON) && (track_data[1] == 0x00))
        //status = MIDI_STATUS_NOTE_OFF;
      MIDI_Event* event = midi_event_new_with_delta_in_frames(0, status, track_data, num_arguments);
      if(event == NULL) return;
      midi_event_set_delta_in_raw_bytes(event, delta_time, num_delta_bytes);
     
      midi_track_append_event(track, event);
      //fprintf(stderr, "orig_size: %i, calculated_size: %i, event_size: %i args:%i type %i\r\n", size, midi_track_num_bytes(track), midi_event_get_num_bytes(event), status, track_data[1]);
      
      track_data += num_arguments;
      num_bytes_read += num_arguments;
    }
  midi_track_attempt_to_resolve_note_on_off_pairs(track);
}

/*-------------------------------------------------------------------------------------*/
MIDI_File* midi_file_new(uint16_t frames_per_beat /*should be 1680 by default*/)
{
  MIDI_File* self = calloc(1, sizeof(*self));
  if(self != NULL)
    {
      self->track_list = list_new();
      if(self->track_list == NULL)
        return midi_file_destroy(self);
        
      self->frames_per_beat = frames_per_beat;
    }
    
  return self;
}

/*-------------------------------------------------------------------------------------*/
MIDI_Track* midi_file_add_empty_tracks(MIDI_File* self, int num_tracks)
{
  MIDI_Track* track = NULL;
  for(; num_tracks>0; num_tracks--)
    {
      if(list_count(self->track_list) >= 0xFFFF)
        return track;
      track = midi_track_new();
      if(track == NULL)
        break;
      list_append_data(self->track_list, track, (list_data_deallocator_t) midi_track_destroy);
    }
  return track;
}

/*-------------------------------------------------------------------------------------*/
void midi_file_append_track(MIDI_File* self, MIDI_Track* track)
{
  list_append_data(midi_file_get_track_list(self), track, (list_data_deallocator_t)midi_track_destroy);
}

/*-------------------------------------------------------------------------------------*/
void midi_file_end_all_tracks(MIDI_File* self)
{
  uint8_t bytes[3];
  bytes[0] = MIDI_STATUS_META_EVENT;
  bytes[1] = MIDI_META_END_OF_TRACK;
  bytes[2] = 0x00; //size of data to follow
  
  MIDI_Track* track;
  list_iterator_t i = list_reset_iterator(self->track_list);
  while((track = list_advance_iterator(self->track_list, &i)) != NULL)
    midi_file_add_byte_array_to_track(self, track, bytes, 3 /*bytes*/, 4/*beats delta*/);
}

/*-------------------------------------------------------------------------------------*/
void midi_file_remove_end_from_all_tracks(MIDI_File* self)
{
  MIDI_Track* track;
  list_iterator_t i = list_reset_iterator(self->track_list);
  while((track = list_advance_iterator(self->track_list, &i)) != NULL)
    {
      List* event_list = midi_track_get_event_list(track);
      unsigned num_events = list_count(event_list);
      
      MIDI_Event* event = list_data_at_index(event_list, num_events-1);
      if((midi_event_get_status_byte(event) == MIDI_STATUS_META_EVENT) &&
         (midi_event_get_argument_at_index(event, 0) == MIDI_META_END_OF_TRACK))
          list_remove_data_at_index(event_list, num_events-1, YES);
    }
}

/*-------------------------------------------------------------------------------------*/
BOOL midi_file_save_with_filename(MIDI_File* self, char* name)
{
  midi_file_end_all_tracks(self);
  FILE* file = fopen(name, "w+");
  if(file == NULL)
    return NO;

  //write header
  if(!midi_file_write_uint_32(file, MIDI_FILE_HEADER_CHUNK_ID))
    goto out;
  if(!midi_file_write_uint_32(file, MIDI_FILE_HEADER_CHUNK_SIZE))
    goto out;
  if(!midi_file_write_uint_16(file, MIDI_FILE_POLYPHONIC_FORMAT_TYPE))
    goto out;
  if(!midi_file_write_uint_16(file, (uint16_t)list_count(self->track_list)))
    goto out;
  if(!midi_file_write_uint_16(file, self->frames_per_beat))
    goto out;

  list_iterator_t i;
  MIDI_Track* track;
  i = list_reset_iterator(self->track_list);
  while((track = list_advance_iterator(self->track_list, &i)) != NULL)
    {
      unsigned num_bytes = midi_track_num_bytes(track);
      
      //write track chunk
      if(!midi_file_write_uint_32(file, MIDI_FILE_TRACK_CHUNK_ID))
        goto out;
      if(!midi_file_write_uint_32(file, num_bytes))
        goto out;
        
      //write track data
      uint8_t* buffer = malloc(num_bytes);
      if(buffer == NULL) goto out;
      midi_track_serialize(track, buffer, num_bytes);
      fwrite(buffer, 1, num_bytes, file);
      free(buffer);
    }
  
  fclose(file);
  midi_file_remove_end_from_all_tracks(self);
  return YES;
  
  out:
    fclose(file);
    midi_file_remove_end_from_all_tracks(self);
    return NO;
    
}

/*
-(void)addNoteOn:(mkMIDIPitch_t)num 
	withChan:(mkMIDIChannel_t)chan 
	     vel:(mkMIDIVelocity_t)vel
afterDurationInQuarters:(float)numQuarters
	 toTrack:(unsigned int)track
{
  unsigned char event[3];
  event[0] = NOTE_ON | chan;
  event[1] = num;
  event[2] = vel;
  [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: numQuarters toTrack: track];
}

-(void)addNoteOff:(mkMIDIPitch_t)num 
	 withChan:(mkMIDIChannel_t)chan 
 afterDurationInQuarters:(float)numQuarters
	  toTrack:(unsigned int)track
{
  unsigned char event[3];
  event[0] = NOTE_OFF | chan;
  event[1] = num;
  event[2] = VELOCITY_OFF;
  [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: numQuarters toTrack: track];
} 

-(void)addNote:(mkMIDIPitch_t)num
      withChan:(mkMIDIChannel_t)chan
	   vel:(mkMIDIVelocity_t)vel
durationInQuarters:(float)numQuarters
       toTrack:(unsigned int)track
{
  [self addNoteOn: num
	withChan: chan
	vel: vel
	afterDurationInQuarters: 0
	toTrack: track];

  [self addNoteOff: num
	withChan: chan
	afterDurationInQuarters: numQuarters
	toTrack: track];
}

-(void)addRestWithDurationInQuarters:(float)numQuarters
			     toTrack:(unsigned int)track
{
  float newValue = [previousRest floatAtIndex: track-1];
  newValue += numQuarters;
  [previousRest setFloat: newValue atIndex: track-1];
}

-(void)addString:(char*)string toTrack:(unsigned int)track withType:(mkMIDIMetaEvent_t)type
{
  MKMIDIEvent* event = [[MKMIDIEvent alloc] init];
  unsigned int i, size = strlen(string);
  [event setDeltaTimeWithNumQuarters: [self getPreviousRestOnTrack: track] framesPerBeat: framesPerBeat];
  [event setStatusByte: META_EVENT];
  [event addArgument: type];
  [event addArgument: size];
  for(i=0; i<size; i++)  [event addArgument: *(string+i)];

  [[trackArray objectAtIndex: track-1] addEvent: event];
}

-(void)addCopyright:(char*)string toTrack:(unsigned int)track
{
  [self addString: string toTrack: track withType: COPYRIGHT];
}

-(void)addText:(char*)string toTrack:(unsigned int)track
{
  [self addString: string toTrack: track withType: INSERT_LYRIC];
}

-(void)addLyric:(char*)string toTrack:(unsigned int)track
{
  [self addString: string toTrack: track withType: INSERT_TEXT];
}

-(void)addTrackName:(char*)string toTrack:(unsigned int)track
{
  [self addString: string toTrack: track withType: TRACK_NAME];
}
-(void)addInstrumentName:(char*)string toTrack:(unsigned int)track
{
  [self addString: string toTrack: track withType: INSTRUMENT_NAME];
}

-(void)addTempoChange:(float)bpm toTrack:(unsigned int)track
{
  unsigned char event[6];
  unsigned int microsecsPerQuarter = 60000000.0 / bpm;
  if(thisSystemIsLittleEndian()) byteSwap32(&microsecsPerQuarter);
  event[0] = META_EVENT;
  event[1] = TEMPO_CHANGE;
  event[2] = 0x03; //numBytesToFollow;
  event[3] = (microsecsPerQuarter >> 8) & 0xFF;
  event[4] = (microsecsPerQuarter >> 16) & 0xFF;
  event[5] = (microsecsPerQuarter >> 24) & 0xFF;

  [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: 0.0 toTrack: track];
}  

-(void)addMeterChangeWithNumerator:(unsigned char)numerator 
		    denominator:(unsigned char)denominator 
	thirtySecondsPerQuarter:(unsigned char) numThirtySeconds
			   toTrack:(unsigned int)track
{
  unsigned char event[7];
  unsigned char logDenominator = 0;
  while(denominator > 1) {denominator = (float)denominator/2.0; logDenominator++;}
  
  event[0] = META_EVENT;
  event[1] = TIME_SIGNATURE;
  event[2] = 4; //numBytes to follow
  event[3] = numerator;
  event[4] = logDenominator;
  event[5] = 24;
  event[6] = numThirtySeconds;

  [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: 0.0 toTrack: track];
}
*/

/*-------------------------------------------------------------------------------------*/
void midi_file_add_byte_array_to_track(MIDI_File* self, MIDI_Track* track, uint8_t* bytes, unsigned num_bytes, double beats_delta)
{
  if(num_bytes == 0) return;
  //beats_delta += [self getPreviousRestOnTrack: track];

  MIDI_Event* event = midi_event_new_with_delta_in_frames(beats_delta*self->frames_per_beat, bytes[0], bytes+1, num_bytes-1);
  //MIDI_Event* event = midi_event_new_with_delta_in_beats(beats_delta, self->frames_per_beat, bytes[0], bytes+1, num_bytes-1);
  if(event != NULL)
    midi_track_append_event(track, event);
}

/*
-(void)addKeyChangeWithNumAccidentals:(char)numAccidentals sharps:(BOOL)sharps major:(BOOL)major toTrack:(unsigned int)track;
{
  if(!sharps) numAccidentals *= -1;
  unsigned char event[5];  
  event[0] = META_EVENT;
  event[1] = KEY_SIGNATURE;
  event[2] = 2; //numBytes to follow
  event[3] = (unsigned char)numAccidentals;
  event[4] = (unsigned char)major;
 
  [self addByteArray: event ofSize: sizeof(event) afterNumQuarters: 0.0 toTrack: track];
}

-(void)quantizeToDuration:(unsigned char)duration
{
  if([trackArray resetIterator])
    {
      do{
	[[trackArray currentObject]quantizeToDuration: duration withFramesPerBeat: framesPerBeat];
      }while([trackArray advanceIterator: 1]);
    }
}

float duration_in_quarters(MIDI_File* self)
{
  float maxDuration = 0, currentDuration;
  if([trackArray resetIterator])
    {
      do{
	currentDuration = [[trackArray currentObject] durationInQuartersWithFramesPerBeat: framesPerBeat];
	if(currentDuration > maxDuration) maxDuration = currentDuration;
      }while([trackArray advanceIterator: 1]);
    }
  return maxDuration;
}
*/

/*-------------------------------------------------------------------------------------*/
unsigned midi_file_get_num_tracks(MIDI_File* self)
{
  return list_count(self->track_list);
}

/*-------------------------------------------------------------------------------------*/
List*        midi_file_get_track_list(MIDI_File* self)
{
  return self->track_list;
}

/*-------------------------------------------------------------------------------------*/
MIDI_Track*  midi_file_get_track_at_index(MIDI_File* self, uint16_t index)
{
  return list_data_at_index(self->track_list, index);
}

/*-------------------------------------------------------------------------------------*/
int   midi_file_get_frames_per_beat(MIDI_File* self)
{
  return self->frames_per_beat;
}

/*
-(void)setFramesPerBeat:(unsigned short int)value
{
  framesPerBeat = value;
}
*/
/*-------------------------------------------------------------------------------------*/
MIDI_File* midi_file_destroy(MIDI_File* self)
{
  if(self != NULL)
    {
      list_destroy(self->track_list, YES);
      free(self);
    }
  return (MIDI_File*) self;
}
