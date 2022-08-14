#include "Loudness.h"
#include "MKAiff.h"
#include "BodePlot.h"


#ifndef YES
#define NO 0
#define YES !NO
#endif

int main(int argc, char* argv[])
{
  if(argc < 2)
    {fprintf(stderr, "which audio files would you like to analyze?\r\n"); return -1;}
  
  fprintf(stderr, "filename\tnum_samples\tsample_rate\tloudenss\tpeak\tpeak_db\trms\trms_db\r\n");
  
  while(--argc > 0)
    {
      ++argv;
      MKAiff* aiff = aiffWithContentsOfFile(*argv);
      if(aiff == NULL)
        {fprintf(stderr, "unable to create aiff object from %s\r\n", *argv); continue;}
      aiffMakeMono(aiff);
      aiffRewindPlayheadToBeginning(aiff);
      double sample_rate = aiffSampleRate(aiff);
      int num_samples = aiffDurationInSamples(aiff);
      float* samples = calloc(num_samples, sizeof(float));
      if(samples == NULL)
        {fprintf(stderr, "unable to calloc %i audio samples\r\n", num_samples); continue;}
      aiffReadFloatingPointSamplesAtPlayhead(aiff, samples, num_samples, aiffYes);
      double peak      = loudenss_get_peak_amplitude(samples, num_samples);
      double rms       = loudenss_get_rms_amplitude(samples, num_samples);
      double peak_db   = loudenss_amplidutde_to_db(peak);
      double rms_db    = loudenss_amplidutde_to_db(rms);
      double loudness  = loudness_calculate(samples, num_samples, sample_rate);

      fprintf(stderr, "%s\t%i\t%f\t%f\t%f\t%f\t%f\t%f\r\n", *argv, num_samples, sample_rate, loudness, peak, peak_db, rms, rms_db);
     
      Bode* bode = bode_new(aiff);
      if(bode == NULL) {perror("Could not make bode object"); continue;}
      
      char* bode_name = NULL;
      asprintf(&bode_name, "%s.png", *argv);

      bode_set_is_log_freq_scale(bode, 1);
      bode_set_min_db (bode, -60);
      bode_set_max_db (bode,  60);
      bode_calculate_spectrum      (bode);
      bode_set_line_width          (bode, 1);
      bode_lifter                  (bode, YES, NO, 10000);
      bode_make_image              (bode, YES, NO);
      bode_save_image_as_png(bode, bode_name);
  
      bode = bode_destroy(bode);
      aiff = aiffDestroy(aiff);
      free(samples);
    }

  return 0;
}
