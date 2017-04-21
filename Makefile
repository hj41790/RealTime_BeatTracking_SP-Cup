C =  decimator.cpp novelty_curve.cpp preprocess.cpp BPMxcorr.cpp fft.cpp

C1D = $(C)  main_test.cpp 
C2D = $(C)  main_mic.cpp

H1D = custom_soundcard.h decimator.h fft.cpp novelty_curve.h preprocess.h BPMxcorr.h type.h timestamp.h

F = -std=c++11 -lfftw

all: main_mic

main_mic: $(C2D) $(H1D)
	sudo modprobe snd-pcm-oss
	g++ -o main $(C2D) $(F)
	ls /dev/dsp*

main_test:	$(C1D) $(H1D)
	g++ -o main_test $(C1D) $(F)

test : test.cpp $(FFT) $(S) $(H1D)
	g++ -o test $(F) test.cpp

clean:
	rm -f main main_test test

