/**
 * @brief 
 * 
 */

#pragma once

#include <stdint.h>


/**
 * @brief structure for AIC signals
 * 
 */

// DATA TYPES ----------------------------------------------------------------------------

const int SAMPLING_FREQUENCY = 400e3;           // sampling frequency (Hz)
const int SAMPLE_SIZE = 8192;   // how many samples to read at once

struct Signal_RE
{   
    const uint16_t Fs_kHz = SAMPLING_FREQUENCY / 1e3;
    const float Ts_us = (1.0 / SAMPLING_FREQUENCY) * 1e6;
    const uint16_t length = SAMPLE_SIZE;
    
    uint16_t util_emitter = 0;
    uint16_t util_receiver = 0;
    
    uint16_t emitter[SAMPLE_SIZE];
    uint16_t receiver[SAMPLE_SIZE];
};

/**
 * @brief parameter structure for AIC signals.
 * Contains all the auxiliary parameters needed to compute AIC.
 */
struct SignalParams
{
    int vth;  // Threshold, in sample value. Should be relative to signal max amplitude.
    int first_vth_surpass_index;  // first sample index signal surpasses vth
    int last_vth_surpass_index;  // last sample index signal surpasses vth

    int left_window_margin;  // margin to the left from fisrt vth surpass
    int right_window_margin;  // margin to the right from first vth surpass

    int window_init;  // window init sample
    int window_end;   // window end sample
    int window_size;  // window size. less or equal to MAX_WINDOW_SIZE

    int max_index;  // index of the sample with max value (first absolute max).
    int max_value;  // value of the first maximum

    int arrival_index;
    int arrival_time_us;
};
