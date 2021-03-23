#include "Matrix.h"
#include "Timestamp.h"
#include "Harmonizer.h"
#include "HarmonizerController.h"

#include <stdio.h>

int main(void)
{
  HarmonizerController* controller = harmonizer_controller_new();
  if(controller == NULL)
    {perror("unable to create Harmonizer Controller"); return -1;}
  
  auPlay((Audio*)controller);
  
  for(;;)
    sleep(1);
  return 0;
}

int _main(void)
{

  unsigned num_tests = 100;
  unsigned a_rows = 2411;
  unsigned a_cols = 1235;
  unsigned b_cols = 1;

  Matrix* a = matrix_new(a_rows, a_cols);
  Matrix* b = matrix_new(a_cols, b_cols);
  Matrix* result_1 = matrix_new(a_rows, b_cols);
  Matrix* result_2 = matrix_new(a_rows, b_cols);
  
  matrix_fill_random_flat(a);
  matrix_fill_random_flat(b);
  
  
  int i;
  
  timestamp_microsecs_t start_1 = timestamp_get_current_time();
  for(i=0;  i<num_tests; i++)
    matrix_multiply(a, b, result_1);
  timestamp_microsecs_t end_1 = timestamp_get_current_time();

  timestamp_microsecs_t start_2 = timestamp_get_current_time();
  for(i=0;  i<num_tests; i++)
    matrix_post_multiply_vector(a, b, result_2);
  timestamp_microsecs_t end_2 = timestamp_get_current_time();
  
  //matrix_print(a);
  //matrix_print(b);
  //matrix_print(result_1);
  //matrix_print(result_2);

  if(matrix_is_pointwise_equal(result_1, result_2))
    fprintf(stderr, "equal\r\n");
  else
    fprintf(stderr, "not equal\r\n");
    
  fprintf(stderr, "speed_ratio: %02f\r\n", (double)(end_1-start_1) / (double)(end_2-start_2));
  
  return 0;
}

