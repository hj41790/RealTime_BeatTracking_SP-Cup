#include "fft.h"

#define SIZE_N 32768

typedef std::complex<float> Complex;
typedef std::valarray<Complex> CArray;

void make_hann();
void preprocessing(CArray & out, float (*finaloutput)[SIZE_N]);
