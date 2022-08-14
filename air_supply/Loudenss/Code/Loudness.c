#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Loudness.h"
#include "DFT.h"

/*-------------------------------------------------------------------*/
double   loudness_hz_to_bark(double hz)
{
  double x1 = 13.0 * atan(hz * (1.0/1315.8));
  double x2 = 3.5  * atan(hz * (1.0/7518.0));
  return x1 + x2;
}

/*-------------------------------------------------------------------*/
double   loudness_fft_index_to_bark         (int fft_n, int index, double sample_rate)
{
  double hz = dft_frequency_of_bin(index, sample_rate, fft_n);
  return loudness_hz_to_bark(hz);
}

/*-------------------------------------------------------------------*/
void loudness_calculate_bark_bands_energy(float* fft, int fft_n, double* bark_bands, int bark_bands_n, double sample_rate)
{
  int i;
  //not really sure if this is correct;
  double highest_bark_freq = 16000; //separate into 24 equal bark bands from 0 to 16kHz
  double highest_fft_index = round(dft_bin_of_frequency(highest_bark_freq, sample_rate, fft_n));
  if(highest_fft_index >= fft_n / 2) highest_fft_index = fft_n / 2;
  double highest_bark_band = loudness_fft_index_to_bark(fft_n, highest_fft_index, sample_rate);
  double one_over_bark_bandwidth = bark_bands_n / highest_bark_band;
  
  memset(bark_bands, 0, bark_bands_n * sizeof(*bark_bands));
  
  for(i=0; i<highest_fft_index; i++)
  {
    int bark_index = (int)(loudness_fft_index_to_bark(fft_n, i, sample_rate) * one_over_bark_bandwidth);
    bark_bands[bark_index] += fft[i] * fft[i];
  }
}

/*-------------------------------------------------------------------*/
double loudness_calculate_total_loudness_from_bark_bands(double* bark_bands, int num_bark_bands)
{
  int i;
  double total_loudness = 0;
  for(i=0; i<num_bark_bands; i++)
    total_loudness += pow(bark_bands[i], 0.23);
  return total_loudness;
}

/*-------------------------------------------------------------------*/
double loudness_calculate(float* samples, int num_samples, double sample_rate)
{
  double result = 0;
  int num_bark_bands = 24;
  int N = dft_smallest_power_of_2_at_least_as_great_as(num_samples);
  float* spectrum = calloc(N, sizeof(*spectrum));
  float* imag     = calloc(N, sizeof(*imag));
  float* window   = calloc(N, sizeof(*window));
  if((spectrum == NULL) || (imag == NULL) || (window == NULL)) return -1;
  double bark_bands[num_bark_bands];
  
  memcpy(spectrum, samples, num_samples * sizeof(*samples));
  dft_init_blackman_window(window, N);
  dft_apply_window(spectrum, window, N);
  dft_real_forward_dft(spectrum, imag, N);
  //loudness_fft_real_dft(samples, num_samples, spectrum, N);
  loudness_calculate_bark_bands_energy(spectrum, N, bark_bands, num_bark_bands, sample_rate);
  result = loudness_calculate_total_loudness_from_bark_bands(bark_bands, num_bark_bands);
  
  free(spectrum);
  free(imag);
  free(window);
  
  return result;
}

/*-------------------------------------------------------------------*/
double loudness_calculate_from_spectrum(float* spectrum, int num_samples, double sample_rate)
{
  int num_bark_bands = 24;
  double bark_bands[num_bark_bands];
  loudness_calculate_bark_bands_energy(spectrum, num_samples, bark_bands, num_bark_bands, sample_rate);
  return loudness_calculate_total_loudness_from_bark_bands(bark_bands, num_bark_bands);
}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
double loudenss_get_peak_amplitude(float* samples, int num_samples)
{
  int i;
  double max = 0;
  
  for(i=0; i<num_samples; i++)
    if(fabs(samples[i]) > max)
      max = fabs(samples[i]);

  return max;
}

/*-------------------------------------------------------------------*/
double loudenss_get_rms_amplitude(float* samples, int num_samples)
{
  int i;
  long double rms = 0;

  for(i=0; i<num_samples; i++)
    rms += samples[i] * samples[i];
  
  rms /= (long double)num_samples;
  rms = sqrt(rms);
  
  return rms;

}

/*-------------------------------------------------------------------*/
double loudenss_amplidutde_to_db(double amp)
{
  return 20 * log10(amp);
}

/*-------------------------------------------------------------------*/
double loudenss_db_to_amplitude(double db)
{
  return pow(10, db/20.0);
}
