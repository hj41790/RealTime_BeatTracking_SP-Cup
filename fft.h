#include <complex>
#include <valarray>

typedef std::complex<float> Complex;
typedef std::valarray<Complex> CArray;

void fft_window(CArray& x);
void fft(CArray& x);
void ifft(CArray& x);

void fft_prepare();
void fft_release();
