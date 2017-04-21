#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

#define  SCERR_NO_PROC_DEVICE            -1    // not exist /proc/device
#define  SCERR_DRIVER_NO_FIND            -2    // cannot find driver from /proc/device
#define  SCERR_NO_SOUNDCARD              -3    // no soundcard
#define  SCERR_NO_MIXER                  -4    // no mixer
#define  SCERR_OPEN_SOUNDCARD            -5    // failed opening soundcard
#define  SCERR_OPEN_MIXER                -6    // failed opening mixer
#define  SCERR_PRIVILEGE_SOUNDCARD       -7    // no privilege for soundcard
#define  SCERR_PRIVILEGE_MIXER           -8    // no privilege for mixer

#define  SCERR_NO_FILE                   -10   // no file
#define  SCERR_NOT_OPEN                  -11   // cannot open file
#define  SCERR_NOT_WAV_FILE              -12   // not WAV file
#define  SCERR_NO_WAV_FORMAT             -13   // not exist WAV format information

#define	 BAD_FILE_DESCRIPTOR		 -14   // wrong descriptor
#define  ERR_WRITE_WAV_HEADER		 -15   // failed wav header

#define  TRUE                 1
#define  DSP_DEVICE_NAME      "/dev/dsp"
#define  MIXER_DEVICE_NAME    "/dev/mixer"
#define  PCM_WAVE_FORMAT      1

typedef  struct
{
	char     RiffID [4] ;
	u_long   RiffSize ;
	char     WaveID [4] ;
	char     FmtID  [4] ;
	u_long   FmtSize ;
	u_short  wFormatTag ;
	u_short  nChannels ;
	u_long   nSamplesPerSec ;
	u_long   nAvgBytesPerSec ;
	u_short  nBlockAlign ;
	u_short  wBitsPerSample ;
	char     DataID [4] ;
	u_long   nDataBytes ;
} wave_header_t;

wave_header_t  wave_header =
{  { 'R', 'I', 'F', 'F' },
	0,
	{ 'W', 'A', 'V', 'E' },
	{ 'f', 'm', 't', ' ' },
	16,
	PCM_WAVE_FORMAT,
	0,
	0,
	0,
	0,
	0,
	{ 'd', 'a', 't', 'a' },
	0
};

int            fd_soundcard     = -1;
int            fd_mixer         = -1;       // volume control file descriptor


//------------------------------------------------------------------------------
// description  : print error message
// parameter	: format     : ex) "%s%d"
//        	  ...        : values for print
//------------------------------------------------------------------------------
void  printx( const char *_format, ...)
{
	const char     *bar  = "- - - - - - - - - - - - - - - - - - - - - - - - - - - -\n";
	va_list   parg;
	int       count;

	printf("%s", bar);

	va_start( parg,_format);
	count = vprintf(_format, parg);
	va_end( parg);

	printf("%s", bar);
}

//------------------------------------------------------------------------------
// description	: set header data
// parameter	: _channels   : 1 = mono
//         	                2 = stereo
//      	  _samplerate : bit rate
//    	          _sampbits   : 16 or 8
//     	 	  _samples    : record tlrks(sec) * _samplerate
// return 	: error code - 0 = success
//------------------------------------------------------------------------------
int  soundcard_set_wav_header(int _channels, u_long _samplerate, int _sampbits, u_long _samples)
{  
	u_long      databytes ;
	u_short     blockalign ;

//	if ( _fd < 0 ) return BAD_FILE_DESCRIPTOR;

	_sampbits   = (_sampbits == 16) ? 16 : 8 ;
	blockalign  = ( (_sampbits == 16) ? 2 : 1) * _channels ;
	databytes   = _samples * (u_long) blockalign ;

	wave_header.RiffSize          = sizeof ( wave_header_t) + databytes - 8 ;
	wave_header.wFormatTag      = PCM_WAVE_FORMAT ;
	wave_header.nChannels       = _channels ;
	wave_header.nSamplesPerSec  = _samplerate ;
	wave_header.nAvgBytesPerSec = _samplerate * (u_long) blockalign ;
	wave_header.nBlockAlign     = blockalign ;
	wave_header.wBitsPerSample  = _sampbits ;
	wave_header.nDataBytes      = databytes;

//	if (  sizeof ( wave_header_t) != write (_fd, &wave_header, sizeof ( wave_header_t)))
//		return ERR_WRITE_WAV_HEADER;

	return 0 ;
};

//------------------------------------------------------------------------------
// description	: get buffer size for playing sound
// return	: soundcard buffer size
//------------------------------------------------------------------------------
int   soundcard_get_buff_size( void)
{
	int   size  = 1024;

	if ( 0 <= fd_soundcard)
	{
		ioctl( fd_soundcard, SNDCTL_DSP_GETBLKSIZE, &size);
		if ( size < 1024 )
			size  = 1024;
	}
	return size;
}

//------------------------------------------------------------------------------
// description	: set soundcard stereo/mono
// parameter	: _enable  - 1 : stereo
//             		     0 : mono
//------------------------------------------------------------------------------
void  soundcard_set_stereo( int _enable)
{
	if ( 0 <= fd_soundcard)    ioctl( fd_soundcard, SNDCTL_DSP_STEREO, &_enable);
}


//------------------------------------------------------------------------------
// description	: set bitrate for soundcard
// parameter	: _rate - bitrate
//------------------------------------------------------------------------------
void  soundcard_set_bit_rate( int _rate)
{
	if ( 0 <= fd_soundcard)    ioctl( fd_soundcard, SNDCTL_DSP_SPEED, &_rate );
}


//------------------------------------------------------------------------------
// description  : set bit size of play sound
// parameter	: _size - sound data bit size
//------------------------------------------------------------------------------
void  soundcard_set_data_bit_size( int _size)
{
	if ( 0 <= fd_soundcard)    ioctl( fd_soundcard, SNDCTL_DSP_SAMPLESIZE, &_size);
}


//------------------------------------------------------------------------------
// description	: set sound card volume
// parameter	: _channel - channel number
// 		  	     headphone volume : SOUND_MIXER_WRITE_VOLUME
//                	     PCM volume : SOUND_MIXER_WRITE_PCM
//                           speaker volume : SOUND_MIXER_WRITE_SPEAKER
//             		     line volume : SOUND_MIXER_WRITE_LINE
//      	             MIC volume : SOUND_MIXER_WRITE_MIC
//     		             CD volume : SOUND_MIXER_WRITE_CD
//         	             PCM2 volume : SOUND_MIXER_WRITE_ALTPCM    default) speacker volume of ESP-MMI
//       	             IGain  volume : SOUND_MIXER_WRITE_IGAIN
//       	             line 1 volume : SOUND_MIXER_WRITE_LINE1
//       	  _left : left or lower value of volume
//      	  _right: right or upper value of volume
//------------------------------------------------------------------------------
void soundcard_set_volume( int _channel, int _left, int _right )
{
	int volume;

	if ( 120 < _left  ) _left  = 120;
	if ( 120 < _right ) _right = 120;

	volume = 256 *_right +_left;
	ioctl( fd_mixer, _channel, &volume );
}


//--------------------------------------------------------------------
// description	: close soundcard
//--------------------------------------------------------------------
void soundcard_close( void)
{
	close( fd_soundcard);
	close( fd_mixer    );
}


//--------------------------------------------------------------------
// description	: open soundcard for use
// parameter	: _name		    : soundcard device name(user input)
//		  _mode - PLAYER    : play only
// 		  _mode - RECODER   : record only
// 		  _mode - AUDIO     : play and record both
// return	: soundcard file descriptor
//--------------------------------------------------------------------
int   soundcard_open(char *_name, int _mode)
{
	const char  *dsp_devname   = (_name==NULL) ? DSP_DEVICE_NAME : _name;
	const char  *mixer_devname = MIXER_DEVICE_NAME;

	// soundcard open
	if ( 0 != access( dsp_devname, W_OK ) )
		return SCERR_PRIVILEGE_SOUNDCARD;

	fd_soundcard = open( dsp_devname,_mode);
	if ( 0 > fd_soundcard)
		return SCERR_OPEN_SOUNDCARD;

	// MIXER open
	if ( 0 != access( mixer_devname, W_OK ) )
		return SCERR_PRIVILEGE_MIXER;

	fd_mixer = open(mixer_devname, O_RDWR|O_NDELAY);
	if ( 0 > fd_soundcard)
		return SCERR_OPEN_MIXER;

	return fd_soundcard;
}
