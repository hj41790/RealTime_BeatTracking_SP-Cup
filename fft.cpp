#include <cstdio>
#include <complex>
#include <iostream>
#include <algorithm>
#include <fftw.h>

#include "fft.h"

const double PI = 3.141592653589793238460;

void fft(CArray& x)
{
	int n = x.size();
	
	fftw_plan p = fftw_create_plan(n, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_complex in[n], out[n];

	for(int i=0; i<n; i++){
		in[i].re = x[i].real();
		in[i].im = x[i].imag();
	}

	fftw_one(p, in, out);

	for(int i=0; i<n; i++)
		x[i] = Complex(c_re(out[i]), c_im(out[i]));
	
	fftw_destroy_plan(p);
}

// inverse fft (in-place)
void ifft(CArray& x)
{
	int n = x.size();
	
	fftw_plan p = fftw_create_plan(n, FFTW_BACKWARD, FFTW_ESTIMATE);
	fftw_complex in[n], out[n];

	for(int i=0; i<n; i++){
		in[i].re = x[i].real();
		in[i].im = x[i].imag();
	}

	fftw_one(p, in, out);

	for(int i=0; i<n; i++)
		x[i] = Complex(c_re(out[i]), c_im(out[i]));

	fftw_destroy_plan(p);
	x /= n;

}


/*
// Cooley–Tukey FFT (in-place)
void fft(CArray& x)
{
	const size_t N = x.size();
	if (N <= 1) return;

	// divide
	CArray even = x[std::slice(0, N/2, 2)];
	CArray  odd = x[std::slice(1, N/2, 2)];

	// conquer
	fft(even);
	fft(odd);

	// combine
	for (size_t k = 0; k < N/2; ++k)
	{
		Complex t = std::polar((float)1.0, (float)(-2 * PI * k / N)) * odd[k];
		x[k    ] = even[k] + t;
		x[k+N/2] = even[k] - t;
	}
}

// inverse fft (in-place)
void ifft(CArray& x)
{

    // conjugate the complex numbers
    x = x.apply(std::conj);

    // forward fft
    fft( x );

    // conjugate the complex numbers again
    x = x.apply(std::conj);

    // scale the numbers
    x /= x.size();
}
*/
