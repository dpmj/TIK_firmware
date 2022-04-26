/* trunk signals
 * struct for organizing signal data captured from ADCs
 */


#ifndef FS
#define FS 500e3            // sampling frequency (Hz)
#endif

#ifndef SAMPLE_SIZE
#define SAMPLE_SIZE 16384   // how many samples to read at once
#endif

struct trunk_signal
{   
    const int Fs_kHz = FS / 1e3;
    const int Ts_us = (int)(float)(1/FS * 1e6);
    const int length = SAMPLE_SIZE;
    int time_us[SAMPLE_SIZE];
    int emitter[SAMPLE_SIZE];
    int receiver[SAMPLE_SIZE];
};

