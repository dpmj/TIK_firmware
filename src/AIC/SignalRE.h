/**
 * @brief 
 * 
 */

#pragma once

#include <stdint.h>


#define SAMPLING_FREQUENCY 500e3            // sampling frequency (Hz)
#define SAMPLE_SIZE 8192   // how many samples to read at once

/**
 * @brief structure for AIC signals
 * 
 */
struct Signal_RE
{   
    const uint16_t Fs_kHz = SAMPLING_FREQUENCY / 1e3;
    const float Ts_us = (float)(1/SAMPLING_FREQUENCY * 1e6);
    const uint16_t length = SAMPLE_SIZE;
    
    uint16_t util = SAMPLE_SIZE;
    uint16_t emitter[SAMPLE_SIZE];
    uint16_t receiver[SAMPLE_SIZE];
};