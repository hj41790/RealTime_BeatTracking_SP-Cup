#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#include <complex>
#include <valarray>
#include <iostream>
#include <cmath>
#include <fftw.h>

#include "decimator.h"
#include "fft.h"
#include "novelty_curve.h"
#include "preprocess.h"
#include "BPMxcorr.h"

#define SIZE 163840

unsigned Microseconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec*1000000 + ts.tv_nsec/1000;
}

int main(){

	unsigned start, end;

	int i, count=0;
	int fd = open("1.wav", O_RDONLY);

	short buf[SIZE] = {0};
	char buffer[100];
	unsigned char sample[2];

	Decimator *decimator = new Decimator();
	decimator->initialize(8820.0, 4096.0, 5);

	CArray data(SIZE_N);
	float signal[6][SIZE_N]={0};

	if(read(fd, buffer, 78)<0){
		printf("reading head error\n");
	}

	for(int i=0; i<SIZE; i++){
		if(read(fd, sample, 2)>0){
			short tmp = (unsigned short)sample[1]<<8 | (unsigned short)sample[0];
			buf[i] = tmp;
			count++;
		}
		else{
			printf("data end : count = %d\n\n", count);
			break;
		}
	}
	printf("data end : count = %d\n\n", count);
	close(fd);

	make_hann();
	printf("make_hann\n");
	
	start = Microseconds();

	float data_deci_before[SIZE]={0};
	float data_deci_after[SIZE/5] = {0};

	for(int i=0; i<SIZE; i++) data_deci_before[i] = buf[i]/32768.0;
	decimator->decimate(data_deci_before, data_deci_after, SIZE/5);

	printf("decimate\n");

	for(int i=0; i<SIZE_N; i++) {
		data[i] = Complex(data_deci_after[i], 0);
//		data[i] = Complex(data_deci_before[i], 0);
	}
	printf("make data\n");

	preprocessing(data, signal);
/*
FILE *fp1 = fopen("signal6.txt","w+");
for(int i=0; i<SIZE_N; i++){
//	printf("i\n");
	fprintf(fp1, "%f\n", signal[5][i]);
}
fclose(fp1);
*/
	printf("preprocessing\n");

	float output[6][SEG_SIZE] = {0};

	for(int i=0; i<6; i++){
		novelty_curve(signal[i], 8820, output[i]);
	}
	

	printf("novelty_curve\n");

	float last_data[SEG_SIZE]={0};
	for(int i=0; i<SEG_SIZE; i++)
		for(int j=0; j<6; j++){
			last_data[i] += output[j][i];
		}

	printf("novelty_curve sum\n");

	int BPM;
	float start_point = BPMxcorr(last_data, &BPM);

	end = Microseconds();

	printf("BPM : %d\nstart_point=%f\n", BPM, start_point);
	printf("time : %f\n", (end-start)/1000000.0);


	for(int i=0; start_point<10; i++){
		printf("%f ", start_point+=(60.0/BPM));
		if(i==4) printf("\n");
	}
	printf("\n");

	return 0;
}
