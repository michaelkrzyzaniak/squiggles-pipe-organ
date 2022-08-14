#include "Util.h"
#include "Timestamp.h"
#include <string.h>

float scalef(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void  scaleb(float* buffer, int numSamples, float min, float max)
{
  float minSample = 1000000/*FLT_MAX*/, maxSample = 0;
  float* b = buffer; 
  int    N = numSamples;
  
  
  while(N--)
    {
      if(*b < minSample)
        minSample = *b;
      if(*b > maxSample)
        maxSample = *b;
      b++;
    }
    
  N = numSamples;
  b = buffer;
  
  while(N-- > 0)
    {
      *b = scalef(*b, minSample, maxSample, min, max);
      b++;
    }
}

/*-----------------------------------------------------*/
#include <libgen.h> //dirname
void util_get_unique_filename(char* directory, char* original_file_name, char* extension, char** result)
{
  asprintf(result, "%s/%s_%llu.%s", directory, basename(original_file_name), timestamp_get_current_time(), extension);
}

/*---------------------------------------------------------------*/
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <fcntl.h>
struct termios oldTerminalAttributes;

void iHateCanonicalInputProcessingIReallyReallyDo(void)
{
  int error;
  struct termios newTerminalAttributes;
  
  int fd = fcntl(STDIN_FILENO,  F_DUPFD, 0);
  
  error = tcgetattr(fd, &(oldTerminalAttributes));
  if(error == -1) {  fprintf(stderr, "Error getting serial terminal attributes\r\n"); return;}
  
  newTerminalAttributes = oldTerminalAttributes;
  
  cfmakeraw(&newTerminalAttributes);
  
  error = tcsetattr(fd, TCSANOW, &newTerminalAttributes);
  if(error == -1) {  fprintf(stderr,  "Error setting serial attributes\r\n"); return; }
}

/*------------------------------------------------------------------------------------*/
void makeStdinCannonicalAgain()
{
  int fd = fcntl(STDIN_FILENO,  F_DUPFD, 0);
  
  if (tcsetattr(fd, TCSANOW, &oldTerminalAttributes) == -1)
    fprintf(stderr,  "Error setting serial attributes\r\n");
}

/*-----------------------------------------------------*/
/*
char* util_basename(char* path)
{
  int len = strlen(path);

  char* base = path;
  int i;
  
  for(i=len-1; i>=0; i--)
    {
      if(path[i] == '.')
        {
          path[i] = '\0';
          break;
        }
    }
  
  len = strlen(path);
  
  for(i=len-1; i>=0; i--)
    {
      if(path[i] == '/')
        {
          base = path + i + 1;
          break;
        }
    }

    return base;
}
*/
