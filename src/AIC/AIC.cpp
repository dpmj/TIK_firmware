/* AKAIKE INFORMATION CRITERION FUNCTION, TO DETECT ARRIVAL OF WEAK SIGNALS.
 * Juan del Pino Mena
 * 2022-03-29
 */



#include "AIC.h"



/**
 * @brief Construct a new AIC::AIC object
 * 
 * @param signals 
 */
AIC::AIC(const Signal_RE *signals)
{
    _signals = signals;
}



/**
 * @brief Finds and returns the index and value of a vector
 * As the two signal vectors have (or at least should have) the exact same length, we can
 * iterate at the same time instead of two times to be more efficient.
 */
void AIC::_find_signal_maxs()
{
    _emitter_params.max_index = 0;
    _emitter_params.max_value = _signals->emitter[0];
    _receiver_params.max_index = 0;
    _receiver_params.max_value = _signals->receiver[0];

    for (int i = 1; i < _signals->length; i++) {
        if (_signals->emitter[i] > _emitter_params.max_value) {
            _emitter_params.max_index = i;
            _emitter_params.max_value = _signals->emitter[i];
        }
        if (_signals->receiver[i] > _receiver_params.max_value) {
            _receiver_params.max_index = i;
            _receiver_params.max_value = _signals->receiver[i];
        }
    }
}



/**
 * @brief Finds first and last samples where signal was triggered.
 * 
 */
void AIC::_find_vth_surpass() {
    bool emitter_triggered = false;
    bool receiver_triggered = false;
    uint16_t i = 0;

    _emitter_params.vth = _emitter_params.max_value * EMITTER_VTH_FACTOR;
    _receiver_params.vth = _receiver_params.max_value * RECEIVER_VTH_FACTOR;

    _emitter_params.first_vth_surpass_index = 0;
    _emitter_params.last_vth_surpass_index = 0;
    _receiver_params.first_vth_surpass_index = 0;
    _receiver_params.last_vth_surpass_index = 0;

    // EMITTER ---------------------------------------------------------------------------

    // Find first trigger (going forward on the vector)
    emitter_triggered = false;
    i = 0;
    while (!emitter_triggered && i < _signals->length) {
        if (_signals->emitter[i] >= _emitter_params.vth) {
            _emitter_params.first_vth_surpass_index = i;
            emitter_triggered = true;
        }
        i++;
    }
    if (!emitter_triggered) {
        // Exception
        // If all the vector has been read but not triggered, there is something wrong.
        // It Could try again with lower vth.
        // This should never happen if emitter is greatly amplified and ADC works fine.
    }

    // Find last trigger (going backwards on the vector)
    emitter_triggered = false;
    i = _signals->length - 1;
    while (!emitter_triggered && i >= 0) {
        if (_signals->emitter[i] >= _emitter_params.vth) {
            _emitter_params.first_vth_surpass_index = i;
            emitter_triggered = true;
        }
        i--;
    }
    
    // RECEIVER --------------------------------------------------------------------------

    // Find first trigger (going forward on the vector)
    receiver_triggered = false;
    i = 0;
    while (!receiver_triggered && i < _signals->length) {
        if (_signals->receiver[i] >= _receiver_params.vth) {
            _receiver_params.first_vth_surpass_index = i;
            receiver_triggered = true;
        }
        i++;
    }
    if (!receiver_triggered) {
        // Exception
        // This is more likely to happen than previous case, as receiver is a much weaker
        // signal, and it will be much less amplified, unsaturated
    }

    // Find last trigger (going backwards on the vector)
    receiver_triggered = false;
    i = _signals->length - 1;
    while (!receiver_triggered && i >= 0) {
        if (_signals->receiver[i] >= _receiver_params.vth) {
            _receiver_params.first_vth_surpass_index = i;
            receiver_triggered = true;
        }
        i--;
    }
}



/**
 * @brief Window selection algorithm
 * 
 */
void AIC::_extract_window() 
{
    AIC::_find_signal_maxs();
    AIC::_find_vth_surpass();

    // start analyzing when we know for sure the emitter has not started yet
    _receiver_params.window_init = _emitter_params.first_vth_surpass_index - LEFT_WINDOW_MARGIN;
    if (_receiver_params.window_init < 0) {
        _receiver_params.window_init = 0;
    }
    // stop analyzing when there is sufficient waveform data
    _receiver_params.window_end = _emitter_params.first_vth_surpass_index + RIGHT_WINDOW_MARGIN;
    if (_receiver_params.window_end >= _signals->length) {
        _receiver_params.window_end = _signals->length - 1;
    }
    _receiver_params.window_size = _receiver_params.window_end - _receiver_params.window_init + 1;
}



/**
 * @brief computes the mean value of a given vector on the specified range
 * 
 * @param vector sample vector
 * @param start start position to include on the mean calculus (inclusive)
 * @param end end position to include on the mean calculus (inclusive)
 * @return int 
 */
uint16_t AIC::_mean(const uint16_t vector[], uint16_t start, uint16_t end)
{
    uint16_t mean = 0;
    for (uint16_t i = start; i <= end; i++) {
        mean += vector[i];
    }
    mean /= (end - start + 1);  // (end - start) == (N - 1)
    return mean;
}



/**
 * @brief computes the sample variance of a given vector on the specified range
 * 
 * @param vector sample vector
 * @param start start position to include on the variance calculus (inclusive)
 * @param end end position to include on the variance calculus (inclusive)
 * @return int
 */
uint16_t AIC::_var(const uint16_t vector[], uint16_t start, uint16_t end)
{
    uint16_t var = 0, aux = 0;
    uint16_t mean = AIC::_mean(vector, start, end);
    for (uint16_t i = start; i <= end; i++) {
        aux = (vector[i] - mean);
        var += aux * aux;
    }
    var /= (end - start);  // (end - start) == (N - 1)
    return var;
}


/**
 * @brief Finds the index of the first absolute minimum value of a given vector
 * 
 * @param vector sample vector
 * @param start start position to include on the minimum search (inclusive)
 * @param end end position to include on the minimum search (inclusive)
 * @return int 
 */
uint16_t AIC::_find_minimum_index(const uint32_t vector[], uint16_t start, uint16_t end)
{   
    uint16_t value = vector[start];
    uint16_t index = 0;
    for (uint16_t i = start + 1; i <= end; i++) {
        if (vector[i] < value) {
            value = vector[i];
            index = i;
        }
    }
    return index;
}


/**
 * @brief Computes the AIC Algorithm and returns the ToF time (delay between 2 signals).
 * 
 */
uint16_t AIC::compute()
{   
    // 'k' has to be always more than 2, and in practice it has to start at a higher value
    uint16_t k = NON_ZERO_START_POINT;
    const uint16_t v_init = _receiver_params.window_init + NON_ZERO_START_POINT;
    const uint16_t v_end = _receiver_params.window_size - NON_ZERO_START_POINT;
    uint16_t delay_samples, delay_time_us;
    

    // EMITTER ---------------------------------------------------------------------------
    // Flank detection

    _emitter_params.arrival_index = _emitter_params.first_vth_surpass_index;
    _emitter_params.arrival_time_us = _signals->Ts_us * _emitter_params.arrival_index;


    // RECEIVER --------------------------------------------------------------------------
    // Akaike Information Criterion function calculus and arrival time selection

    // Possible optimization: it does not have to compute all the vector length, only 
    // until we know there is signal on the receiver: the emitter_trigger point.
    // This method also provides a narrower minimum decision window with less error.
    // Change condition 'i < v_end' for 'i < _receiver_params.first_vth_surpass_index'

    for (uint16_t i = v_init; i <= v_end; i++)
    {
        _receiver_akaike_func[k] = k * log(AIC::_var(_signals->receiver, v_init, v_init + k))
                                   + (_receiver_params.window_size - k - 1)
                                   * (log(AIC::_var(_signals->receiver, v_init + k + 1, v_end)));
        k++;
    }

    _receiver_params.arrival_index = AIC::_find_minimum_index(_receiver_akaike_func, v_init, v_end);

    delay_samples = _receiver_params.arrival_index - _emitter_params.arrival_index;
    delay_time_us = _signals->Ts_us * delay_samples;

    return delay_time_us;
}