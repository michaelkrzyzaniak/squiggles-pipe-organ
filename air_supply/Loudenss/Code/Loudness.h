#ifndef __MFCC__
#define __MFCC__ 1

#if defined(__cplusplus)
extern "C"{
#endif   //(__cplusplus)

/*-------------------------------------------------------------------*/
double loudness_calculate(float* samples, int num_samples, double sample_rate);

/*-------------------------------------------------------------------*/
double loudness_calculate_from_spectrum(float* spectrum, int num_samples, double sample_rate);

/*-------------------------------------------------------------------*/
double loudenss_get_peak_amplitude(float* samples, int num_samples);

/*-------------------------------------------------------------------*/
double loudenss_get_rms_amplitude(float* samples, int num_samples);

/*-------------------------------------------------------------------*/
double loudenss_amplidutde_to_db(double amp);

/*-------------------------------------------------------------------*/
double loudenss_db_to_amplitude(double db);

#if defined(__cplusplus)
}
#endif   //(__cplusplus)

#endif   // __MFCC__
