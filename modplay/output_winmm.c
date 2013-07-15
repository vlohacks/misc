/* Native WinMM Output is broken & unfinished. 
 * - gave it up and used portaudio instead
 */
#include "output_winmm.h"

#include <windows.h>
#include <mmsystem.h>

WAVEFORMATEX wf;
WAVEHDR whdr;
HWAVEOUT hWaveOut;

int16_t * winmm_output_buffer;
int winmm_output_buffer_size = 1024;
int winmm_output_buffer_pos;

int output_winmm_init(int output_argc, char ** output_argv)
{
    winmm_output_buffer = (int16_t *)malloc(sizeof(int16_t) * winmm_output_buffer_size);
    winmm_output_buffer_pos = 0;

    wf.wFormatTag = WAVE_FORMAT_PCM;
    wf.nChannels = 2;
    wf.nSamplesPerSec = 44100;
    wf.nAvgBytesPerSec = 44100;
    wf.nBlockAlign = wf.nChannels * wf.wBitsPerSample / 8;
    wf.wBitsPerSample = 16;
    wf.cbSize = 0;

    whdr.lpData = winmm_output_buffer;
    whdr.dwBufferLength = winmm_output_buffer_size * sizeof(int16_t);
    whdr.dwFlags = WHDR_BEGINLOOP;
    whdr.dwLoops = 0;

    waveOutOpen(&hWaveOut,WAVE_MAPPER,&wf,0,0,CALLBACK_NULL);
    waveOutPrepareHeader(hWaveOut,&whdr,sizeof(whdr));
    whdr.dwFlags=WHDR_BEGINLOOP|WHDR_ENDLOOP|WHDR_PREPARED;
    
}

int output_winmm_cleanup()
{
    waveOutUnprepareHeader(hWaveOut, &whdr, sizeof(whdr));
    waveOutClose(hWaveOut);
}

int output_winmm_write (float l, float r)
{
    winmm_output_buffer[winmm_output_buffer_pos] = (int16_t)(l * 32767);
    winmm_output_buffer[winmm_output_buffer_pos + 1] = (int16_t)(r * 32767);
    //snd_pcm_writei(playback_handle, output_buffer, 2);
    //snd_pcm_prepare(playback_handle);
    //return 2;
    winmm_output_buffer_pos += 2;
    if (winmm_output_buffer_pos > (winmm_output_buffer_size) ) {
        waveOutWrite(hWaveOut, &whdr, sizeof(whdr));
        output_buffer_pos = 0;
    }        
}
