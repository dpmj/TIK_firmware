/* AKAIKE INFORMATION CRITERION FUNCTION, TO DETECT ARRIVAL OF WEAK SIGNALS.
 * Juan del Pino Mena
 * 2022-03-29
 */

#pragma once

#include <stdint.h>
#include <math.h> // for log()
#include "SignalRE.h"



// PARAMETERS ----------------------------------------------------------------------------

#define MAX_WINDOW_SIZE 8000
#define NON_ZERO_START_POINT 10

#define EMITTER_VTH_FACTOR 0.75
#define RECEIVER_VTH_FACTOR 0.75

#define LEFT_WINDOW_MARGIN 200
#define RIGHT_WINDOW_MARGIN 1000


// DATA TYPES ----------------------------------------------------------------------------

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






// CLASS HEADERS -------------------------------------------------------------------------

/**
 * @brief AIC computation library.
 * 
 */
class AIC
{
private:

    // Attributes

    const struct Signal_RE *_signals;

    SignalParams _emitter_params;
    SignalParams _receiver_params;

    uint32_t _receiver_akaike_func[SAMPLE_SIZE];


    // Methods

    void _find_signal_maxs();  // find absolute signal maximas
    void _find_vth_surpass();  // find threshold surpasses
    void _extract_window();    // compute AIC window limits
    uint16_t _mean(const uint16_t vector[], uint16_t start, uint16_t end);  // compute a vector mean
    uint16_t _var(const uint16_t vector[], uint16_t start, uint16_t end);   // compute a vector variance
    uint16_t _find_minimum_index(const uint32_t vector[], uint16_t start, uint16_t end);


public:
    AIC(const struct Signal_RE *signals);  // Constructor

    // uint16_t delay_time_us;  // delay between signals in microseconds
    // uint16_t delay_samples;  // delay in number of samples

    uint16_t compute();      // compute the AIC algorithm
    
};
