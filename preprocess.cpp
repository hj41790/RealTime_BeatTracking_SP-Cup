#include <iostream>
#include <cstdio>
#include <complex>
#include <valarray>
#include <cmath>

#include "preprocess.h"

CArray hann(SIZE_N);

void make_hann(){
	float a;
	float b;

	FILE *real = fopen("window_real.txt", "r");
	FILE *imag = fopen("window_imag.txt", "r");

	for(int i=0; i<SIZE_N; i++){
		fscanf(real, "%f", &a);
		fscanf(imag, "%f", &b);

		hann[i] = Complex(a, b);
	}

	fclose(real);
	fclose(imag);
}


void preprocessing(CArray & out, float (*finaloutput)[SIZE_N])
{
	int bandlimits[7] = { 0,200,400,800,1600,3200,4096 };
	int nbands = 6;
	
	//filterbank
	CArray dft = out;
	fft(dft);
	int n=dft.size();
	int bl[nbands];
	int br[nbands];
	for(int i=0;i<nbands;++i)
	{
		bl[i]=floor(bandlimits[i]*n*0.5/bandlimits[nbands])+1;
		br[i]=floor(bandlimits[i+1]*n*0.5/bandlimits[nbands]);
	}
	br[nbands-1] = floor(n*0.5);

	CArray output1[nbands];
	for(int i=0;i<nbands;++i)
	{
		output1[i].resize(n);
		for(int j=bl[i]-1;j<br[i];++j)
		{
			output1[i][j]=dft[j];
			output1[i][n-j-1]=dft[n-j-1];
		}
	}
	output1[0][0]=Complex();
/*
	FILE *fp = fopen("abcd.txt", "w");
	for(int i=0; i<SIZE_N; i++)
		fprintf(fp, "%f\n", abs(output1[0][i]));
	fclose(fp);

	for(int i=0; i<6; i++){
		ifft(output1[i]);
//		printf("ifft\n");
		for(int j=0; j<SIZE_N; j++)
			finaloutput[i][j] = abs(output1[i][j]);
	}
*/


//	printf("filterbank\n");

	//hwindow
	CArray wave[nbands]={CArray(n),CArray(n),CArray(n),CArray(n),CArray(n),CArray(n)};
	for(int i=0;i<nbands;++i)
	{   
		ifft(output1[i]);
		for(int j=0;j<n;++j)
		{
			wave[i][j]=Complex(fabs(output1[i][j].real()),0);
		}
		fft(wave[i]);
	}
//	printf("in");

//	CArray output2[nbands]={CArray(n),CArray(n),CArray(n),CArray(n),CArray(n),CArray(n)};
	for(int i=0;i<nbands;++i)
	{
		for(int j=0;j<n;++j)
		{
//			output2[i][j]=wave[i][j]*hann[j];
			wave[i][j]*=hann[j];
		}
//		ifft(output2[i]);
		ifft(wave[i]);
	}

//	printf("hwindow\n");

	//diffrect
	for(int i=0;i<nbands;++i)
	{
		for(int j=4;j<n;++j)
		{
//			float d = output2[i][j].real()-output2[i][j-1].real();
			float d = wave[i][j].real()-wave[i][j-1].real();
			if(d>0)
			{
				finaloutput[i][j]=d;
			}
			else
			{
				finaloutput[i][j]=0;
			}
		}
	}

//	printf("diffrect\n");


}
