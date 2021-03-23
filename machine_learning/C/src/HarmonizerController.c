/*
 *  Synth.h
 *  Make weird noises
 *
 *  Made by Michael Krzyzaniak at Arizona State University's
 *  School of Arts, Media + Engineering in Spring of 2013
 *  mkrzyzan@asu.edu
 */

#include "HarmonizerController.h"
#include "Harmonizer.h"
#include "Beep.h"
//#include "Timestamp.h"

HarmonizerController* harmonizer_controller_destroy (HarmonizerController* self);
void harmonizer_controller_note_changed_callback(void* SELF, int midi_note);

/*--------------------------------------------------------------------*/
struct OpaqueHarmonizerControllerStruct
{

  AUDIO_GUTS               ;
  Harmonizer* harmonizer   ;
  Beep*  beep              ;
  //Robot*  robot            ;
};

/*--------------------------------------------------------------------*/
HarmonizerController* harmonizer_controller_new ()
{
  HarmonizerController* self = (HarmonizerController*) auAlloc(sizeof(*self), harmonizer_controller_audio_callback, NO, 2);
  if(self != NULL)
    {
      self->destroy = (Audio* (*)(Audio*))harmonizer_controller_destroy;

      self->harmonizer = harmonizer_new("matrices");
      if(self->harmonizer == NULL)
        return (HarmonizerController*)auDestroy((Audio*)self);
        
      self->beep = beep_new();
      if(self->beep == NULL)
        return (HarmonizerController*)auDestroy((Audio*)self);
        
      harmonizer_set_note_changed_callback(self->harmonizer, harmonizer_controller_note_changed_callback, self);

      /*
      self->robot = robot_new(mic_message_recd_from_robot, self);
      if(self->robot == NULL)
        return (HarmonizerController*)auDestroy((Audio*)self)
        
      
      sleep(1);
      robot_send_message(self->robot, robot_cmd_get_firmware_version);
      */
      //there should be a play callback that I can intercept and do this there.
      auPlay((Audio*)self->beep);
    }
  return self;
}

/*--------------------------------------------------------------------*/
HarmonizerController* harmonizer_controller_destroy (HarmonizerController* self)
{
  if(self != NULL)
    {
      harmonizer_destroy(self->harmonizer);
      //robot_destroy(self->robot);
      auDestroy((Audio*)self->beep);
    }
    
  return (HarmonizerController*) NULL;
}

/*--------------------------------------------------------------------*/
/*
Robot*            harmonizer_controller_get_robot (HarmonizerController* self)
{
  return self->robot;
}
*/

/*--------------------------------------------------------------------*/
/*
void harmonizer_controller_message_recd_from_robot(void* self, char* message, robot_arg_t args[], int num_args)
{
  //robot_debug_print_message(message, args, num_args);
  switch(robot_hash_message(message))
    {
      case robot_hash_reply_firmware_version:
        if(num_args == 2)
          fprintf(stderr, "Teensy is running firmware version %i.%i\r\n", robot_arg_to_int(&args[0]), robot_arg_to_int(&args[1]));
        break;
      
      default: break;
    }
}
*/

/*--------------------------------------------------------------------*/
void harmonizer_controller_note_changed_callback(void* SELF, int midi_note)
{
  HarmonizerController* self = SELF;
  beep_set_note(self->beep, midi_note);
}

/*--------------------------------------------------------------------*/
int harmonizer_controller_audio_callback(void* SELF, auSample_t* buffer, int num_frames, int num_channels)
{
  HarmonizerController* self = SELF;
  int frame, channel;
  auSample_t samp = 0;
  
  
  //mix to mono without correcting amplitude
  for(frame=0; frame<num_frames; frame++)
    {
      samp = 0;
      for(channel=0; channel<num_channels; channel++)
        samp += buffer[frame * num_channels + channel];
      buffer[frame] = samp;
  }

  harmonizer_process_audio(self->harmonizer, buffer, num_frames);

  return  num_frames;
}

