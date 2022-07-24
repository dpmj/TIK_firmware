/* AKAIKE INFORMATION CRITERION FUNCTION, TO DETECT ARRIVAL OF WEAK SIGNALS.
 * Juan del Pino Mena
 * 2022-03-29
 */

#pragma once

#include <stdint.h>
#include <math.h> // for log()

#include "SIGNALS/SignalRE.h"  // Signal structure



// PARAMETERS ----------------------------------------------------------------------------

#define MAX_WINDOW_SIZE 8000
#define NON_ZERO_START_POINT 10

#define EMITTER_VTH_FACTOR 0.75
#define RECEIVER_VTH_FACTOR 0.75

#define LEFT_WINDOW_MARGIN 200
#define RIGHT_WINDOW_MARGIN 1000






// CLASS HEADERS -------------------------------------------------------------------------

/**
 * @brief AIC computation library.
 * 
 */
class AIC
{
private:

    // Attributes

    struct Signal_RE *_signals;

    SignalParams _emitter_params;
    SignalParams _receiver_params;

    uint32_t _receiver_akaike_func[SAMPLE_SIZE];


    // Methods

    void _reorganize_signals();  // reorder ADC samples 
    void _find_signal_maxs();  // find absolute signal maximums
    void _find_vth_surpass();  // find threshold surpasses
    void _extract_window();    // compute AIC window limits
    uint16_t _mean(const uint16_t vector[], uint16_t start, uint16_t end);  // compute a vector mean
    uint16_t _var(const uint16_t vector[], uint16_t start, uint16_t end);   // compute a vector variance
    uint16_t _find_minimum_index(const uint32_t vector[], uint16_t start, uint16_t end);


public:
    AIC(struct Signal_RE *signals);  // Constructor

    // uint16_t delay_time_us;  // delay between signals in microseconds
    // uint16_t delay_samples;  // delay in number of samples

    uint16_t compute();      // compute the AIC algorithm
    
};
