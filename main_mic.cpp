#include <iostream>
#include <cstdio>
#include <fcntl.h>
#include <csignal>
#include <cmath>
#include <thread>

#include "custom_soundcard.h"
#include "decimator.h"
#include "fft.h"
#include "novelty_curve.h"
#include "preprocess.h"
#include "BPMxcorr.h"
#include "timestamp.h"
#include "type.h"

#define BUFSIZE 8192

using namespace std;

// global variable
char 		sound_buff[BUFSIZ];	
CArray 		data(SIZE_N);
msg_pipe 	p_msg;

int 		c_fd;
int 		p_fd[2];

FILE *fp;

// func header
void handler_sigint(int signo);
void handler_sigpipe(int signo);
void calculate();
void childprocess();



int main(int argc, char** argv){

	// argument check
	if(argc<3) {
		printx( "%s%s",
				"Error : wrong input\n",
				"Usage : sudo ./main [device_name] [txt_filename]\n");
		exit(1);
	}

	int fd_soundcard;
	int sample_size = 16;
	int bit_rate = 8820;
	//	int bit_rate = 44100;

	// SIGPIPE handler register
	signal(SIGINT, handler_sigint);	
	signal(SIGPIPE, handler_sigpipe);

	make_hann();
	fp = fopen(argv[2], "w");

	pipe(p_fd);
	if((c_fd=fork())==0) childprocess();

	// open sound card device
	printf("Audio : open soundcard\n");
	fd_soundcard = soundcard_open(argv[1], O_RDONLY|O_NONBLOCK);

	switch( fd_soundcard)
	{
		case  SCERR_NO_PROC_DEVICE       :  printx( "Error : not exist /proc/device\n");
		case  SCERR_NO_SOUNDCARD         :  printx( "Error : cannot find driver from /proc/device\n");
		case  SCERR_NO_MIXER             :  printx( "Error : no mixer\n");
		case  SCERR_PRIVILEGE_SOUNDCARD  :  printx( "Error : no privilege for soundcard\n");
		case  SCERR_PRIVILEGE_MIXER      :  printx( "Error : no privilege for mixer\n");
		case  SCERR_OPEN_SOUNDCARD       :  printx( "Error : failed opening soundcard\n");
		case  SCERR_OPEN_MIXER           :  printx( "Error : failed opening mixer\n");
	}

	if(fd_soundcard<0){
		soundcard_close();
		exit(1);
	}

	soundcard_set_volume(SOUND_MIXER_WRITE_ALTPCM, 120, 120);
	soundcard_set_volume(SOUND_MIXER_WRITE_MIC   , 80, 80);
	soundcard_set_volume(SOUND_MIXER_WRITE_IGAIN , 60, 60);

	soundcard_set_stereo(0);
	soundcard_set_data_bit_size(sample_size);
	soundcard_set_bit_rate(bit_rate);








	int count = 0;
	int s_size, second=0, read_s_size=0;
	printf("Audio : get data\n");
	p_msg.start_time = Microseconds();	

	write(p_fd[1], &p_msg, sizeof(p_msg));
	while(TRUE)
	{

		s_size = read(fd_soundcard, sound_buff+read_s_size, BUFSIZE-read_s_size);
		if(s_size>0){
			read_s_size += s_size;
			if(read_s_size==BUFSIZE){

				for(int i=0; i<BUFSIZE/2; i++){
					short tmp = (unsigned short)sound_buff[2*i+1]<<8 | (unsigned short)sound_buff[2*i];
					data[second*4096+i] = Complex((float)tmp/SIZE_N, 0);
					//printf("%f\n", data[second*4096+i].real());

//					count++;
				}
				read_s_size = 0;
				second++;

				if(second==8){
					write(p_fd[1], &p_msg, sizeof(p_msg));
			
//					printf("%d  %f\n", count, (Microseconds()-p_msg.start_time)/1000000.0);
					p_msg.start_time = Microseconds();
					second=0;
//					count = 0;

					calculate();

				}

			}

		}


	}

	// never reachable
	soundcard_close();
	return 0;

}

void calculate(){

	float signal_data[6][SIZE_N];
	float output_data[6][SIZE_N];

	// preprocess & novelty_curve
	preprocessing(data, signal_data);

	for(int i=0; i<6; i++) 
		novelty_curve(signal_data[i], 8820, output_data[i]);

	// novelty_curve sum
	float last_data[SEG_SIZE] = {0};
	for(int i=0; i<SEG_SIZE; i++)
		for(int j=0; j<6; j++)
			last_data[i] += output_data[j][i];

	// BPMxcorr
	p_msg.start_point = BPMxcorr(last_data, &p_msg.BPM);

	// send to pipe
//	write(p_fd[1], &p_msg, sizeof(p_msg));

}

void childprocess(){

	int SIZE = sizeof(msg_pipe);
	int p_size, read_p_size=0;

	int count=0, all_count=0;

	unsigned start_time=0;
	unsigned base_time=0;
	float 	 start_point=0;

	float total_time=0;
	float diff;

	float SPB=0;
	float max_time = 3.7152;

	float bit_seq[40]={0};

	read(p_fd[0], &p_msg, SIZE);
	fcntl(p_fd[0], F_SETFL, O_NONBLOCK);

	while(true){
		
		diff = (Microseconds()-base_time)/1000000.0 - total_time;
//		printf("%d  bit_seq : %f  now : %f \n",all_count, bit_seq[count], diff);
		if(all_count>1 && diff >= bit_seq[count]){
			printf("%f\n", bit_seq[count] + total_time);
			fprintf(fp, "%f\r\n", bit_seq[count] + total_time);
			count++;
		}

		if((p_size=read(p_fd[0], &p_msg+read_p_size, SIZE-read_p_size))>0){
			read_p_size += p_size;
			if(read_p_size==SIZE){

//				printf("BPM=%d start=%f \n", p_msg.BPM, p_msg.start_point);

				if(all_count>=0) total_time += max_time;
				if(all_count==0) base_time = p_msg.start_time;
		
				start_time  = p_msg.start_time;
				start_point = p_msg.start_point;
				SPB = 60.0/p_msg.BPM;

				memset(bit_seq, 0, sizeof(bit_seq));
				bit_seq[0] = start_point;
//				printf("%.4f\n", bit_seq[0]);
				for(int i=1;;i++){
					bit_seq[i] = bit_seq[i-1] + SPB;
					if(bit_seq[i] > max_time) break;
//					printf("%.4f\n", bit_seq[i]);
				}
//				printf("\n");

				read_p_size = 0;
				count = 0;
				all_count++;
			}
		}
	}

	printf("childprocess exit\n");

}


void handler_sigint(int signo){
	printf("\n- - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");
	printf("SIGINT : program terminated\n");

	fclose(fp);
	close(p_fd[0]);
	close(p_fd[1]);

	//close soundcard
	soundcard_close();

	printf("        : close soundcard\n");
	printf("        : exit program\n");
	printf("- - - - - - - - - - - - - - - - - - - - - - - - - - - -\n");

	exit(1);
}

void handler_sigpipe(int signo){
	printf("SIGPIPE\n");
}
