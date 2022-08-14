#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define TWO_PI 6.283185307179586

#ifndef BOOL
#define BOOL int
#endif
#ifndef NO
#define NO   0
#define YES  (!NO)
#endif

#define CONSTRAIN(x, MIN, MAX) ((x) = ((x) < (MIN)) ? (MIN) : ((x) > (MAX)) ? (MAX) : (x))
float scalef(float x, float in_min, float in_max, float out_min, float out_max);
void  scaleb(float* buffer, int numSamples, float min, float max);
void  util_get_unique_filename(char* directory, char* orig_file_name, char* extension, char** result);
void  iHateCanonicalInputProcessingIReallyReallyDo(void);
void  makeStdinCannonicalAgain();
//char* basename(char* path);
