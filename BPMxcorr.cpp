#include <iostream>
#include <cmath>
#include <cstdio>
#include <valarray>

#include "novelty_curve.h"

float BPMxcorr(float *data, int* BPM){
	
	float novelty_time_unit = 0.058;

	int bpm = 0;
	int location;
	float max = -1;

	for(int j=20; j<=150; j++){
	
		float beat_pre_time_unit = 60.0/j;
		
		int beat_time_interval = (int)roundf(beat_pre_time_unit/novelty_time_unit);
	
		int local_location = 0;
		float local_max = -1;
		float sum = 0;
		for(int i=0; i<beat_time_interval; i++){
			
			sum = 0.0;
			for(int k=i; k<SEG_SIZE; k+=beat_time_interval)
				sum += data[k];

			sum = fabs(sum);
			if(sum > max){
				local_max = sum;
				local_location = i;
			}

		}

		if(local_max>max){
			bpm = j;
			location = local_location;
			max = local_max;
		}

	}
	
	float beat_time_unit = 60.0/bpm;
	float start_point = location*novelty_time_unit;

	*BPM = bpm;
	return start_point;
}

