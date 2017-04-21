#include <iostream>
#include <cstdio>
#include <cmath>
#include <complex>

#include "novelty_curve.h"
#include "fft.h"

using namespace std;

void read_complex(complex<float> **);
void novelty_curve(float*, int, float*);
/*
int main(){

	float output[SEG_SIZE];
	float signal[DATA_SIZE];

	for(int i=0; i<DATA_SIZE; i++)
		scanf("%f", &signal[i]);

	novelty_curve(signal, 44100, output);

	for(int i=0; i<SEG_SIZE; i++){
		printf("%f ", output[i]);
//		if((i+1)%10==0) printf("\n");
	}

	return 0;
}
*/
void read_complex(Complex stft[SEG_SIZE][FFT_SIZE]){
	
	FILE *real = fopen("real.txt", "r");
	FILE *imag = fopen("imag.txt", "r");

	float a, b;
	for(int j=0; j<FFT_SIZE; j++){
		for(int i=0; i<SEG_SIZE; i++){
			fscanf(real, "%f", &a);
			fscanf(imag, "%f", &b);
			Complex tmp(a, b);
			stft[i][j] = tmp;
		}
	}

	fclose(real);
	fclose(imag);

}

void novelty_curve(float *signal, int fs, float *output){

	Complex stft[SEG_SIZE][FFT_SIZE];
	float amp_freq[SEG_SIZE][FFT_SIZE];
	float diff_freq[SEG_SIZE][FFT_SIZE];

	CArray window_tmp(WINDOW_SIZE);
	Complex max(-9,0);

//	float thresh_num = 0.0001593;
//	Complex thresh(0.0001593, 0);
	// stft

	for(int i=0, idx=0; i<DATA_SIZE && idx<SEG_SIZE; i+=OVERLAP_SIZE, idx++){
		for(int j=0; j<WINDOW_SIZE; j++)
			window_tmp[j] = Complex(signal[i+j] * WINDOW[j], 0);
		
		// fft
		fft(window_tmp);

		for(int j=0; j<FFT_SIZE; j++){
			stft[idx][j] = window_tmp[j];
			if(abs(stft[idx][j]) > abs(max)) max = stft[idx][j];
		}
	}
	// thresh
	for(int i=0; i<SEG_SIZE; i++){
		for(int j=0; j<FFT_SIZE; j++){
			stft[i][j] /= max;
//			if(abs(stft[i][j]) < thresh_num) stft[i][j] = thresh;
		}
	}


	// detection func
	for(int i=0; i<SEG_SIZE; i++)
		for(int j=0; j<FFT_SIZE; j++){
			amp_freq[i][j] = abs(stft[i][j]);
		}

	for(int i=0; i<FFT_SIZE; i++)
		diff_freq[0][i] = amp_freq[0][i];

	for(int i=1; i<SEG_SIZE; i++)
		for(int j=0; j<FFT_SIZE; j++)
			diff_freq[i][j] = amp_freq[i][j]- amp_freq[i-1][j];
		

	for(int i=0; i<SEG_SIZE; i++){
		float sum = 0;
		for(int j=0; j<FFT_SIZE; j++){
			diff_freq[i][j] = diff_freq[i][j]>0 ? diff_freq[i][j]*diff_freq[i][j]:0;
			sum += diff_freq[i][j];
		}

		output[i] = sum;
	}
}
