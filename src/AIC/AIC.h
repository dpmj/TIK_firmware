/* AKAIKE INFORMATION CRITERION FUNCTION, TO DETECT ARRIVAL OF WEAK SIGNALS.
 * Juan del Pino Mena
 * 2022-03-29
 */

#pragma once

#include <math.h> // for log()
#include "../ADCSampler/trunk_signal.h"



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
struct signal_params
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

    const trunk_signal *_signals;

    signal_params _emitter_params;
    signal_params _receiver_params;

    int _receiver_akaike_func[SAMPLE_SIZE];


    // Methods

    void AIC::_find_signal_maxs();  // find absolute signal maximas
    void AIC::_find_vth_surpass();  // find threshold surpasses
    void AIC::_extract_window();    // compute AIC window limits
    int AIC::_mean(const int vector[], int start, int end);  // compute a vector mean
    int AIC::_var(const int vector[], int start, int end);   // compute a vector variance
    int AIC::_find_minimum_index(const int vector[], int start, int end);


public:
    AIC::AIC(const trunk_signal *signals);  // Constructor

    int delay_time_us;  // delay between signals in microseconds
    int delay_samples;  // delay in number of samples

    int AIC::compute();      // compute the AIC algorithm
    
};
