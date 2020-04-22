#include <complex.h>
#include <fftw3.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Compile with the following arguments:
 *      gcc fftw3_r2c_usage.c -o fftw3_r2c_usage -lfftw3 -lm
 *
 * Make sure that you installed fftw3 in your machine first.
 */

int main(int argc, char *argv[])
{
  int N = 9;
  double in[] = { 5, 4, 3, 2, 0, 0, 0, 0, 0 };	/* Example input */

  fftw_complex *out; /* Output */
  fftw_plan p; /* Plan */

  /*
   * Size of output is (N / 2 + 1) because the other remaining items are
   * redundant, in the sense that they are complex conjugate of already
   * computed ones.
   *
   * CASE SIZE 6 (even):
   * [real 0][complex 1][complex 2][real 3][conjugate of 2][conjugate of 1]
   *
   * CASE SIZE 5 (odd):
   * [real 0][complex 1][complex 2][conjugate of 2][conjugate of 1]
   *
   * In both cases the items following the first N/2+1 are redundant.
   */
  out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * (N / 2 + 1));

  /*
   * An fftw plan cares only about the size of in and out,
   * not about actual values. Can (and should) be re-used.
   */
  p = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

  /*
   * Execute the dft as indicated by the plan
   */
  fftw_execute(p);

  /*
   * Print the N/2+1 complex values computed by the DFT function.
   */
  int i;
  for (i = 0; i < N / 2 + 1; i++) {
    printf("out[%d] = {%f, %fi}\n", i, creal(out[i]), cimag(out[i]));
  }

  /*
   * Clean routine
   */
  fftw_destroy_plan(p);
  fftw_free(out);

  return 1;
}
